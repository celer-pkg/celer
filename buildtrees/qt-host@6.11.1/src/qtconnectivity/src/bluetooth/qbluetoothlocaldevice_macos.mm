// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "darwin/btconnectionmonitor_p.h"
#include "qbluetoothlocaldevice_p.h"
#include "qbluetoothlocaldevice.h"
#include "darwin/btdevicepair_p.h"
#include "darwin/btdelegates_p.h"
#include "darwin/btutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qstring.h>
#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmap.h>


#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>
#include <CoreBluetooth/CoreBluetooth.h>

#include <algorithm>

QT_BEGIN_NAMESPACE
class QBluetoothLocalDevicePrivate;
QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(QDarwinBluetoothStateMonitor) : NSObject <CBCentralManagerDelegate>

@property (strong, nonatomic) CBCentralManager *manager;
@property (assign, nonatomic) QT_PREPEND_NAMESPACE(QBluetoothLocalDevicePrivate) *localDevicePrivate;

- (instancetype)initWith:(QT_PREPEND_NAMESPACE(QBluetoothLocalDevicePrivate) *)localDevicePrivate;
- (CBManagerState)currentState;
- (void)startMonitoring;
- (void)stopMonitoring;

@end

QT_BEGIN_NAMESPACE

class QBluetoothLocalDevicePrivate : public DarwinBluetooth::PairingDelegate,
                                     public DarwinBluetooth::ConnectionMonitor
{
    friend class QBluetoothLocalDevice;
    Q_DECLARE_PUBLIC(QBluetoothLocalDevice)
public:
    typedef QBluetoothLocalDevice::Pairing Pairing;

    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *, const QBluetoothAddress & =
                                 QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    bool isValid() const;
    void requestPairing(const QBluetoothAddress &address, Pairing pairing);
    Pairing pairingStatus(const QBluetoothAddress &address) const;

    void bluetoothStateChanged(CBManagerState newState);

private:

    // PairingDelegate:
    void connecting(void *pair) override;
    void requestPIN(void *pair) override;
    void requestUserConfirmation(void *pair,
                                 BluetoothNumericValue) override;
    void passkeyNotification(void *pair,
                             BluetoothPasskey passkey) override;
    void error(void *pair, IOReturn errorCode) override;
    void pairingFinished(void *pair) override;

    // ConnectionMonitor
    void deviceConnected(const QBluetoothAddress &deviceAddress) override;
    void deviceDisconnected(const QBluetoothAddress &deviceAddress) override;

    void emitPairingFinished(const QBluetoothAddress &deviceAddress, Pairing pairing, bool queued);
    void emitError(QBluetoothLocalDevice::Error error, bool queued);

    void unpair(const QBluetoothAddress &deviceAddress);

    DarwinBluetooth::ObjCScopedPointer<QT_MANGLE_NAMESPACE(QDarwinBluetoothStateMonitor)> bluetoothStateMonitor;
    QBluetoothLocalDevice::HostMode hostMode = QBluetoothLocalDevice::HostMode::HostPoweredOff;

    QBluetoothLocalDevice *q_ptr;

    using HostController = DarwinBluetooth::ObjCScopedPointer<IOBluetoothHostController>;
    HostController hostController;

    using PairingRequest = DarwinBluetooth::ObjCStrongReference<DarwinBTClassicPairing>;
    using RequestMap = QMap<QBluetoothAddress, PairingRequest>;

    RequestMap pairingRequests;
    DarwinBluetooth::ObjCScopedPointer<DarwinBTConnectionMonitor> connectionMonitor;
    QList<QBluetoothAddress> discoveredDevices;
};

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                                           const QBluetoothAddress &address) :
        q_ptr(q)
{
    registerQBluetoothLocalDeviceMetaType();
    Q_ASSERT_X(q, Q_FUNC_INFO, "invalid q_ptr (null)");

    QT_BT_MAC_AUTORELEASEPOOL;

    using namespace DarwinBluetooth;

    ObjCScopedPointer<IOBluetoothHostController> defaultController([IOBluetoothHostController defaultController],
                                                                   RetainPolicy::doInitialRetain);
    if (!defaultController) {
        qCCritical(QT_BT_DARWIN) << "failed to init a host controller object";
        return;
    }

    if (!address.isNull()) {
        NSString *const hciAddress = [defaultController addressAsString];
        if (!hciAddress) {
            qCCritical(QT_BT_DARWIN) << "failed to obtain an address";
            return;
        }

        BluetoothDeviceAddress iobtAddress = {};
        if (IOBluetoothNSStringToDeviceAddress(hciAddress, &iobtAddress) != kIOReturnSuccess) {
            qCCritical(QT_BT_DARWIN) << "invalid local device's address";
            return;
        }

        if (address != DarwinBluetooth::qt_address(&iobtAddress)) {
            qCCritical(QT_BT_DARWIN) << "invalid local device's address";
            return;
        }
    }

    defaultController.swap(hostController);
    // This one is optional, if it fails to initialize, we do not care at all.
    connectionMonitor.reset([[DarwinBTConnectionMonitor alloc] initWithMonitor:this],
                            DarwinBluetooth::RetainPolicy::noInitialRetain);
    // Set the initial host mode
    if ([hostController powerState])
        hostMode = QBluetoothLocalDevice::HostConnectable;
    else
        hostMode = QBluetoothLocalDevice::HostPoweredOff;

    // Start monitoring for bluetooth state changes
    bluetoothStateMonitor.reset([[QT_MANGLE_NAMESPACE(QDarwinBluetoothStateMonitor) alloc] initWith:this],
                                  DarwinBluetooth::RetainPolicy::doInitialRetain);
    [bluetoothStateMonitor startMonitoring];
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    [bluetoothStateMonitor stopMonitoring];
    [connectionMonitor stopMonitoring];
}

void QBluetoothLocalDevicePrivate::bluetoothStateChanged(CBManagerState state)
{
    Q_Q(QBluetoothLocalDevice);
    qCDebug(QT_BT_DARWIN) << "Bluetooth state changed to" << state;
    // States other than 'powered ON' and 'powered OFF' are ambiguous to map
    // unto Qt HostModes. For example lack of permissions might temporarily
    // generate an 'unauthorized' state irrespective of bluetooth power state.
    QBluetoothLocalDevice::HostMode mode;
    if (state == CBManagerState::CBManagerStatePoweredOff)
        mode = QBluetoothLocalDevice::HostPoweredOff;
    else if (state == CBManagerState::CBManagerStatePoweredOn)
        mode = QBluetoothLocalDevice::HostConnectable;
    else
        return;

    if (hostMode != mode) {
        hostMode = mode;
        emit q->hostModeStateChanged(hostMode);
    }
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return hostController;
}

void QBluetoothLocalDevicePrivate::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    Q_ASSERT_X(isValid(), Q_FUNC_INFO, "invalid local device");
    Q_ASSERT_X(!address.isNull(), Q_FUNC_INFO, "invalid device address");

    using DarwinBluetooth::device_with_address;
    using DarwinBluetooth::ObjCStrongReference;
    using DarwinBluetooth::RetainPolicy;

    // That's a really special case on OS X.
    if (pairing == QBluetoothLocalDevice::Unpaired)
        return unpair(address);

    QT_BT_MAC_AUTORELEASEPOOL;

    if (pairing == QBluetoothLocalDevice::AuthorizedPaired)
        pairing = QBluetoothLocalDevice::Paired;

    RequestMap::iterator pos = pairingRequests.find(address);
    if (pos != pairingRequests.end()) {
        if ([pos.value() isActive]) // Still trying to pair, continue.
            return;

        // 'device' is autoreleased:
        IOBluetoothDevice *const device = [pos.value() targetDevice];
        if ([device isPaired]) {
            emitPairingFinished(address, pairing, true);
        } else if ([pos.value() start] != kIOReturnSuccess) {
            qCCritical(QT_BT_DARWIN) << "failed to start a new pairing request";
            emitError(QBluetoothLocalDevice::PairingError, true);
        }
        return;
    }

    // That's a totally new request ('Paired', since we are here).
    // Even if this device is paired (not by our local device), I still create a pairing request,
    // it'll just finish with success (skipping any intermediate steps).
    PairingRequest newRequest([[DarwinBTClassicPairing alloc] initWithTarget:address delegate:this],
                              RetainPolicy::noInitialRetain);
    if (!newRequest) {
        qCCritical(QT_BT_DARWIN) << "failed to allocate a new pairing request";
        emitError(QBluetoothLocalDevice::PairingError, true);
        return;
    }

    pos = pairingRequests.insert(address, newRequest);
    const IOReturn result = [newRequest start];
    if (result != kIOReturnSuccess) {
        pairingRequests.erase(pos);
        qCCritical(QT_BT_DARWIN) << "failed to start a new pairing request";
        emitError(QBluetoothLocalDevice::PairingError, true);
    }
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevicePrivate::pairingStatus(const QBluetoothAddress &address)const
{
    Q_ASSERT_X(isValid(), Q_FUNC_INFO, "invalid local device");
    Q_ASSERT_X(!address.isNull(), Q_FUNC_INFO, "invalid address");

    using DarwinBluetooth::device_with_address;
    using DarwinBluetooth::ObjCStrongReference;

    QT_BT_MAC_AUTORELEASEPOOL;

    RequestMap::const_iterator it = pairingRequests.find(address);
    if (it != pairingRequests.end()) {
        // All Obj-C objects are autoreleased.
        IOBluetoothDevice *const device = [it.value() targetDevice];
        if (device && [device isPaired])
            return QBluetoothLocalDevice::Paired;
    } else {
        // Try even if device was not paired by this local device ...
        const ObjCStrongReference<IOBluetoothDevice> device(device_with_address(address));
        if (device && [device isPaired])
            return QBluetoothLocalDevice::Paired;
    }

    return QBluetoothLocalDevice::Unpaired;
}

void QBluetoothLocalDevicePrivate::connecting(void *pair)
{
    // TODO: why unused and if cannot be used - remove?
    Q_UNUSED(pair);
}

void QBluetoothLocalDevicePrivate::requestPIN(void *pair)
{
    // TODO: why unused and if cannot be used - remove?
    Q_UNUSED(pair);
}

void QBluetoothLocalDevicePrivate::requestUserConfirmation(void *pair, BluetoothNumericValue intPin)
{
    // TODO: why unused and if cannot be used - remove?
    Q_UNUSED(pair);
    Q_UNUSED(intPin);
}

void QBluetoothLocalDevicePrivate::passkeyNotification(void *pair, BluetoothPasskey passkey)
{
    // TODO: why unused and if cannot be used - remove?
    Q_UNUSED(pair);
    Q_UNUSED(passkey);
}

void QBluetoothLocalDevicePrivate::error(void *pair, IOReturn errorCode)
{
    Q_UNUSED(pair);
    Q_UNUSED(errorCode);

    emitError(QBluetoothLocalDevice::PairingError, false);
}

void QBluetoothLocalDevicePrivate::pairingFinished(void *generic)
{
    auto pair = static_cast<DarwinBTClassicPairing *>(generic);
    Q_ASSERT_X(pair, Q_FUNC_INFO, "invalid pairing request (nil)");

    const QBluetoothAddress &deviceAddress = [pair targetAddress];
    Q_ASSERT_X(!deviceAddress.isNull(), Q_FUNC_INFO,
               "invalid target address");

    emitPairingFinished(deviceAddress, QBluetoothLocalDevice::Paired, false);
}

void QBluetoothLocalDevicePrivate::deviceConnected(const QBluetoothAddress &deviceAddress)
{
    if (!discoveredDevices.contains(deviceAddress))
        discoveredDevices.append(deviceAddress);

    QMetaObject::invokeMethod(q_ptr, "deviceConnected", Qt::QueuedConnection,
                              Q_ARG(QBluetoothAddress, deviceAddress));
}

void QBluetoothLocalDevicePrivate::deviceDisconnected(const QBluetoothAddress &deviceAddress)
{
    QList<QBluetoothAddress>::iterator devicePos =std::find(discoveredDevices.begin(),
                                                            discoveredDevices.end(),
                                                            deviceAddress);

    if (devicePos != discoveredDevices.end())
        discoveredDevices.erase(devicePos);

    QMetaObject::invokeMethod(q_ptr, "deviceDisconnected", Qt::QueuedConnection,
                              Q_ARG(QBluetoothAddress, deviceAddress));
}

void QBluetoothLocalDevicePrivate::emitError(QBluetoothLocalDevice::Error error, bool queued)
{
    if (queued) {
        QMetaObject::invokeMethod(q_ptr, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error, error));
    } else {
        emit q_ptr->errorOccurred(QBluetoothLocalDevice::PairingError);
    }
}

void QBluetoothLocalDevicePrivate::emitPairingFinished(const QBluetoothAddress &deviceAddress,
                                                       Pairing pairing, bool queued)
{
    Q_ASSERT_X(!deviceAddress.isNull(), Q_FUNC_INFO, "invalid target device address");
    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    if (queued) {
        QMetaObject::invokeMethod(q_ptr, "pairingFinished", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothAddress, deviceAddress),
                                  Q_ARG(QBluetoothLocalDevice::Pairing, pairing));
    } else {
        emit q_ptr->pairingFinished(deviceAddress, pairing);
    }
}

void QBluetoothLocalDevicePrivate::unpair(const QBluetoothAddress &deviceAddress)
{
    Q_ASSERT_X(!deviceAddress.isNull(), Q_FUNC_INFO,
               "invalid target address");

    emitPairingFinished(deviceAddress, QBluetoothLocalDevice::Unpaired, true);
}

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this))
{
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, address))
{
}

QBluetoothLocalDevice::~QBluetoothLocalDevice()
{
    delete d_ptr;
}

bool QBluetoothLocalDevice::isValid() const
{
    return d_ptr->isValid();
}


QString QBluetoothLocalDevice::name() const
{
    QT_BT_MAC_AUTORELEASEPOOL;

    if (isValid()) {
        if (NSString *const nsn = [d_ptr->hostController nameAsString])
            return QString::fromNSString(nsn);
        qCCritical(QT_BT_DARWIN) << Q_FUNC_INFO << "failed to obtain a name";
    }

    return QString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    QT_BT_MAC_AUTORELEASEPOOL;

    if (isValid()) {
        if (NSString *const nsa = [d_ptr->hostController addressAsString])
            return QBluetoothAddress(DarwinBluetooth::qt_address(nsa));

        qCCritical(QT_BT_DARWIN) << Q_FUNC_INFO << "failed to obtain an address";
    } else {
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "invalid local device";
    }

    return QBluetoothAddress();
}

void QBluetoothLocalDevice::powerOn()
{
    if (!isValid())
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "invalid local device";
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    Q_UNUSED(mode);

    if (!isValid())
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "invalid local device";
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    Q_D(const QBluetoothLocalDevice);
    if (!isValid())
        return HostPoweredOff;

    auto state = [d->bluetoothStateMonitor currentState];
    // If the monitored state is unknown or ambiguous, use the HCI state directly.
    // Otherwise use the monitored state. We can't use HCI state always, because there can
    // be a significant delay from "monitored state change" to "HCI state change", causing
    // handlers of hostModeStateChanged() signal to perceive conflicting results (signal
    // parameter value vs. what this getter returns).
    if (state == CBManagerState::CBManagerStatePoweredOff)
        return HostPoweredOff;
    else if (state == CBManagerState::CBManagerStatePoweredOn)
        return HostConnectable;

    return [d->hostController powerState] ? HostConnectable : HostPoweredOff;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    QT_BT_MAC_AUTORELEASEPOOL;

    QList<QBluetoothAddress> connectedDevices;

    // Take the devices known to IOBluetooth to be paired and connected first:
    NSArray *const pairedDevices = [IOBluetoothDevice pairedDevices];
    for (IOBluetoothDevice *device in pairedDevices) {
        if ([device isConnected]) {
            const QBluetoothAddress address(DarwinBluetooth::qt_address([device getAddress]));
            if (!address.isNull())
                connectedDevices.append(address);
        }
    }

    // Add devices, discovered by the connection monitor:
    connectedDevices += d_ptr->discoveredDevices;
    // Find something more elegant? :)
    // But after all, addresses are integers.
    std::sort(connectedDevices.begin(), connectedDevices.end());
    connectedDevices.erase(std::unique(connectedDevices.begin(),
                                       connectedDevices.end()),
                           connectedDevices.end());

    return connectedDevices;
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    QList<QBluetoothHostInfo> localDevices;

    QBluetoothLocalDevice defaultAdapter;
    if (!defaultAdapter.isValid() || defaultAdapter.address().isNull()) {
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO <<"no valid device found";
        return localDevices;
    }

    QBluetoothHostInfo deviceInfo;
    deviceInfo.setName(defaultAdapter.name());
    deviceInfo.setAddress(defaultAdapter.address());

    localDevices.append(deviceInfo);

    return localDevices;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    if (!isValid())
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "invalid local device";

    if (!isValid() || address.isNull()) {
        d_ptr->emitError(PairingError, true);
        return;
    }

    DarwinBluetooth::qt_test_iobluetooth_runloop();

    return d_ptr->requestPairing(address, pairing);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(const QBluetoothAddress &address) const
{
    if (!isValid())
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "invalid local device";

    if (!isValid() || address.isNull())
        return Unpaired;

    return d_ptr->pairingStatus(address);
}

QT_END_NAMESPACE

@implementation QT_MANGLE_NAMESPACE(QDarwinBluetoothStateMonitor)

- (instancetype)initWith:(QT_PREPEND_NAMESPACE(QBluetoothLocalDevicePrivate) *)localDevicePrivate
{
    if ((self = [super init])) {
        self.manager = nil;
        self.localDevicePrivate = localDevicePrivate;
    }
    return self;
}

- (void)startMonitoring
{
    if (self.manager != nil)
        return;
    self.manager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
}

- (void)stopMonitoring
{
    if (self.manager == nil)
        return;
    self.manager.delegate = nil;
    self.manager = nil;
}

- (CBManagerState)currentState
{
    Q_ASSERT(self.manager);
    return self.manager.state;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)aManager
{
    Q_ASSERT(self.manager);
    Q_ASSERT(self.localDevicePrivate);
    Q_ASSERT(self.manager == aManager);
    self.localDevicePrivate->bluetoothStateChanged(aManager.state);
}

@end
