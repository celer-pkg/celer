// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothdeviceinfo.h"
#include "btledeviceinquiry_p.h"
#include "qbluetoothuuid.h"
#include "btnotifier_p.h"
#include "btutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qdebug.h>
#include <QtCore/qendian.h>
#include <QtCore/qlist.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

QBluetoothUuid qt_uuid(NSUUID *nsUuid)
{
    if (!nsUuid)
        return QBluetoothUuid();

    uuid_t uuidData = {};
    [nsUuid getUUIDBytes:uuidData];
    QUuid::Id128Bytes qtUuidData = {};
    std::copy(uuidData, uuidData + 16, qtUuidData.data);
    return QBluetoothUuid(qtUuidData);
}

const int timeStepMS = 100;
const int powerOffTimeoutMS = 30000;

struct AdvertisementData {
    // That's what CoreBluetooth has:
    // CBAdvertisementDataLocalNameKey
    // CBAdvertisementDataTxPowerLevelKey
    // CBAdvertisementDataServiceUUIDsKey
    // CBAdvertisementDataServiceDataKey
    // CBAdvertisementDataManufacturerDataKey
    // CBAdvertisementDataOverflowServiceUUIDsKey
    // CBAdvertisementDataIsConnectable
    // CBAdvertisementDataSolicitedServiceUUIDsKey

    // For now, we "parse":
    QString localName;
    QList<QBluetoothUuid> serviceUuids;
    QHash<quint16, QByteArray> manufacturerData;
    QHash<QBluetoothUuid, QByteArray> serviceData;
    // TODO: other keys probably?
    AdvertisementData(NSDictionary *AdvertisementData);
};

AdvertisementData::AdvertisementData(NSDictionary *advertisementData)
{
    if (!advertisementData)
        return;

    // ... constant CBAdvertisementDataLocalNameKey ...
    // NSString containing the local name of a peripheral.
    NSObject *value = [advertisementData objectForKey:CBAdvertisementDataLocalNameKey];
    if (value && [value isKindOfClass:[NSString class]])
        localName = QString::fromNSString(static_cast<NSString *>(value));

    // ... constant CBAdvertisementDataServiceUUIDsKey ...
    // A list of one or more CBUUID objects, representing CBService UUIDs.

    value = [advertisementData objectForKey:CBAdvertisementDataServiceUUIDsKey];
    if (value && [value isKindOfClass:[NSArray class]]) {
        NSArray *uuids = static_cast<NSArray *>(value);
        for (CBUUID *cbUuid in uuids)
            serviceUuids << qt_uuid(cbUuid);
    }

    NSDictionary *advdict = [advertisementData objectForKey:CBAdvertisementDataServiceDataKey];
    if (advdict) {
        [advdict enumerateKeysAndObjectsUsingBlock:^(CBUUID *key, NSData *val, BOOL *) {
            serviceData.insert(qt_uuid(key), QByteArray::fromNSData(static_cast<NSData *>(val)));
        }];
    }

    value = [advertisementData objectForKey:CBAdvertisementDataManufacturerDataKey];
    if (value && [value isKindOfClass:[NSData class]]) {
        QByteArray data = QByteArray::fromNSData(static_cast<NSData *>(value));
        manufacturerData.insert(qFromLittleEndian<quint16>(data.constData()), data.mid(2));
    }
}

}

QT_END_NAMESPACE

QT_USE_NAMESPACE

@interface  DarwinBTLEDeviceInquiry (PrivateAPI)
- (void)stopScanSafe;
- (void)stopNotifier;
@end

@implementation DarwinBTLEDeviceInquiry
{
    LECBManagerNotifier *notifier;
    ObjCScopedPointer<CBCentralManager> manager;

    QList<QBluetoothDeviceInfo> devices;
    LEInquiryState internalState;
    int inquiryTimeoutMS;

    QT_PREPEND_NAMESPACE(DarwinBluetooth)::GCDTimer elapsedTimer;
}

-(id)initWithNotifier:(LECBManagerNotifier *)aNotifier
{
    if (self = [super init]) {
        Q_ASSERT(aNotifier);
        notifier = aNotifier;
        internalState = InquiryStarting;
        inquiryTimeoutMS = DarwinBluetooth::defaultLEScanTimeoutMS;
    }

    return self;
}

- (void)dealloc
{
    [self stopScanSafe];
    [manager setDelegate:nil];
    [elapsedTimer cancelTimer];
    [self stopNotifier];
    [super dealloc];
}

- (void)timeout:(id)sender
{
    Q_UNUSED(sender);

    if (internalState == InquiryActive) {
        [self stopScanSafe];
        [manager setDelegate:nil];
        internalState = InquiryFinished;
        Q_ASSERT(notifier);
        emit notifier->discoveryFinished();
    } else if (internalState == InquiryStarting) {
        // This is interesting on iOS only, where the system shows an alert
        // asking to enable Bluetooth in the 'Settings' app. If not done yet
        // (after 30 seconds) - we consider this as an error.
        [manager setDelegate:nil];
        internalState = ErrorPoweredOff;
        Q_ASSERT(notifier);
        emit notifier->CBManagerError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
    }
}

- (void)startWithTimeout:(int)timeout
{
    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT(leQueue);
    inquiryTimeoutMS = timeout;
    manager.reset([[CBCentralManager alloc] initWithDelegate:self queue:leQueue],
                  DarwinBluetooth::RetainPolicy::noInitialRetain);
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

    if (central != manager)
        return;

    if (internalState != InquiryActive && internalState != InquiryStarting)
        return;

    Q_ASSERT(notifier);

    using namespace DarwinBluetooth;

    const auto state = central.state;
    if (state == CBManagerStatePoweredOn) {
        if (internalState == InquiryStarting) {
            internalState = InquiryActive;

            if (inquiryTimeoutMS > 0) {
                [elapsedTimer cancelTimer];
                elapsedTimer.reset([[DarwinBTGCDTimer alloc] initWithDelegate:self], RetainPolicy::noInitialRetain);
                [elapsedTimer startWithTimeout:inquiryTimeoutMS step:timeStepMS];
            }

            // ### Qt 6.x: remove the use of env. variable, as soon as a proper public API is in place.
            bool envOk = false;
            const int env = qEnvironmentVariableIntValue("QT_BLUETOOTH_SCAN_ENABLE_DUPLICATES", &envOk);
            if (envOk && env) {
                [manager scanForPeripheralsWithServices:nil
                 options:@{CBCentralManagerScanOptionAllowDuplicatesKey : @YES}];
            } else {
                [manager scanForPeripheralsWithServices:nil options:nil];
            }
        } // Else we ignore.
    } else if (state == CBManagerStateUnsupported) {
        if (internalState == InquiryActive) {
            [self stopScanSafe];
            // Not sure how this is possible at all,
            // probably, can never happen.
            internalState = ErrorPoweredOff;
            emit notifier->CBManagerError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
        } else {
            internalState = ErrorLENotSupported;
            emit notifier->LEnotSupported();
        }
        [manager setDelegate:nil];
    } else if (state == CBManagerStateUnauthorized) {
        if (internalState == InquiryActive)
            [self stopScanSafe];
        internalState = ErrorNotAuthorized;
        emit notifier->CBManagerError(QBluetoothDeviceDiscoveryAgent::MissingPermissionsError);
        [manager setDelegate:nil];
    } else if (state == CBManagerStatePoweredOff) {

#ifndef Q_OS_MACOS
        if (internalState == InquiryStarting) {
            // On iOS a user can see at this point an alert asking to
            // enable Bluetooth in the "Settings" app. If a user does so,
            // we'll receive 'PoweredOn' state update later.
            // No change in internalState. Wait for 30 seconds.
            [elapsedTimer cancelTimer];
            elapsedTimer.reset([[DarwinBTGCDTimer alloc] initWithDelegate:self], RetainPolicy::noInitialRetain);
            [elapsedTimer startWithTimeout:powerOffTimeoutMS step:300];
            return;
        }
#else
            Q_UNUSED(powerOffTimeoutMS);
#endif // Q_OS_MACOS
        [elapsedTimer cancelTimer];
        [self stopScanSafe];
        [manager setDelegate:nil];
        internalState = ErrorPoweredOff;
        // On macOS we report PoweredOffError and our C++ owner will delete us
        // (here we're kwnon as 'self'). Connection is Qt::QueuedConnection so we
        // are apparently safe to call -stopNotifier after the signal.
        emit notifier->CBManagerError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
        [self stopNotifier];
    } else {
        // The following two states we ignore (from Apple's docs):
        //"
        // -CBCentralManagerStateUnknown
        // The current state of the central manager is unknown;
        // an update is imminent.
        //
        // -CBCentralManagerStateResetting
        // The connection with the system service was momentarily
        // lost; an update is imminent. "
        // Wait for this imminent update.
    }

#pragma clang diagnostic pop
}

- (void)stopScanSafe
{
    // CoreBluetooth warns about API misused if we call stopScan in a state
    // other than powered on. Hence this 'Safe' ...
    if (!manager)
        return;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

    if (internalState == InquiryActive) {
        const auto state = manager.get().state;
        if (state == CBManagerStatePoweredOn)
            [manager stopScan];
    }

#pragma clang diagnostic pop
}

- (void)stopNotifier
{
    if (notifier) {
        notifier->disconnect();
        notifier->deleteLater();
        notifier = nullptr;
    }
}

- (void)stop
{
    [self stopScanSafe];
    [manager setDelegate:nil];
    [elapsedTimer cancelTimer];
    [self stopNotifier];
    internalState = InquiryCancelled;
}

- (void)centralManager:(CBCentralManager *)central
        didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary *)advertisementData
        RSSI:(NSNumber *)RSSI
{
    using namespace DarwinBluetooth;

    if (central != manager)
        return;

    if (internalState != InquiryActive)
        return;

    if (!notifier)
        return;

    QBluetoothUuid deviceUuid;

    if (!peripheral.identifier) {
        qCWarning(QT_BT_DARWIN) << "peripheral without NSUUID";
        return;
    }

    deviceUuid = DarwinBluetooth::qt_uuid(peripheral.identifier);

    if (deviceUuid.isNull()) {
        qCWarning(QT_BT_DARWIN) << "no way to address peripheral, QBluetoothUuid is null";
        return;
    }

    const AdvertisementData qtAdvData(advertisementData);
    QString name(qtAdvData.localName);
    if (!name.size() && peripheral.name)
        name = QString::fromNSString(peripheral.name);

    // TODO: fix 'classOfDevice' (0 for now).
    QBluetoothDeviceInfo newDeviceInfo(deviceUuid, name, 0);
    if (RSSI)
        newDeviceInfo.setRssi([RSSI shortValue]);

    if (qtAdvData.serviceUuids.size())
        newDeviceInfo.setServiceUuids(qtAdvData.serviceUuids);

    const QList<quint16> keysManufacturer = qtAdvData.manufacturerData.keys();
    for (quint16 key : keysManufacturer)
        newDeviceInfo.setManufacturerData(key, qtAdvData.manufacturerData.value(key));

    const QList<QBluetoothUuid> keysService = qtAdvData.serviceData.keys();
    for (QBluetoothUuid key : keysService)
        newDeviceInfo.setServiceData(key, qtAdvData.serviceData.value(key));

    // CoreBluetooth scans only for LE devices.
    newDeviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    emit notifier->deviceDiscovered(newDeviceInfo);
}

@end
