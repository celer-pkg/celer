// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2016 Javier S. Pedro <maemo@javispedro.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergycontroller_darwin_p.h"
#include "darwin/btperipheralmanager_p.h"
#include "qlowenergyserviceprivate_p.h"
#include "darwin/btcentralmanager_p.h"
#include "darwin/btutility_p.h"
#include "darwin/uistrings_p.h"

#include "qlowenergyservicedata.h"
#include "qbluetoothlocaldevice.h"
#include "qbluetoothdeviceinfo.h"
#include "qlowenergycontroller.h"
#include "qbluetoothuuid.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qpermissions.h>

QT_BEGIN_NAMESPACE

namespace {

typedef QSharedPointer<QLowEnergyServicePrivate> ServicePrivate;

// Convenience function, can return a smart pointer that 'isNull'.
ServicePrivate qt_createLEService(QLowEnergyControllerPrivateDarwin *controller, CBService *cbService, bool included)
{
    Q_ASSERT_X(controller, Q_FUNC_INFO, "invalid controller (null)");
    Q_ASSERT_X(cbService, Q_FUNC_INFO, "invalid service (nil)");

    CBUUID *const cbUuid = cbService.UUID;
    if (!cbUuid) {
        qCDebug(QT_BT_DARWIN) << "invalid service, UUID is nil";
        return ServicePrivate();
    }

    const QBluetoothUuid qtUuid(DarwinBluetooth::qt_uuid(cbUuid));
    if (qtUuid.isNull()) // Conversion error is reported by qt_uuid.
        return ServicePrivate();

    ServicePrivate newService(new QLowEnergyServicePrivate);
    newService->uuid = qtUuid;
    newService->setController(controller);

    if (included)
        newService->type |= QLowEnergyService::IncludedService;

    // TODO: isPrimary is ... always 'NO' - to be investigated.
    /*
    if (!cbService.isPrimary) {
        // Our guess included/not was probably wrong.
        newService->type &= ~QLowEnergyService::PrimaryService;
        newService->type |= QLowEnergyService::IncludedService;
    }
    */
    return newService;
}

typedef QList<QBluetoothUuid> UUIDList;

UUIDList qt_servicesUuids(NSArray *services)
{
    QT_BT_MAC_AUTORELEASEPOOL;

    if (!services || !services.count)
        return UUIDList();

    UUIDList uuids;

    for (CBService *s in services)
        uuids.append(DarwinBluetooth::qt_uuid(s.UUID));

    return uuids;
}

} // unnamed namespace

QLowEnergyControllerPrivateDarwin::QLowEnergyControllerPrivateDarwin()
{
    void registerQLowEnergyControllerMetaType();
    registerQLowEnergyControllerMetaType();
    qRegisterMetaType<QLowEnergyHandle>("QLowEnergyHandle");
    qRegisterMetaType<QSharedPointer<QLowEnergyServicePrivate>>();
}

QLowEnergyControllerPrivateDarwin::~QLowEnergyControllerPrivateDarwin()
{
    if (const auto leQueue = DarwinBluetooth::qt_LE_queue()) {
        if (role == QLowEnergyController::CentralRole) {
            const auto manager = centralManager.getAs<DarwinBTCentralManager>();
            dispatch_sync(leQueue, ^{
                [manager detach];
            });
        } else {
            const auto manager = peripheralManager.getAs<DarwinBTPeripheralManager>();
            dispatch_sync(leQueue, ^{
                [manager detach];
            });
        }
    }
}

bool QLowEnergyControllerPrivateDarwin::isValid() const
{
    return centralManager || peripheralManager;
}

void QLowEnergyControllerPrivateDarwin::init()
{
    // We have to override the 'init', it's pure virtual in the base.
    // Just creating a central or peripheral should not trigger any
    // error yet.
}

bool QLowEnergyControllerPrivateDarwin::lazyInit()
{
    using namespace DarwinBluetooth;

    if (peripheralManager || centralManager)
        return true;

    if (qApp->checkPermission(QBluetoothPermission{}) != Qt::PermissionStatus::Granted) {
        qCWarning(QT_BT_DARWIN,
                  "Use of Bluetooth LE must be explicitly requested by the application.");
        setError(QLowEnergyController::MissingPermissionsError);
        return false;
    }

    std::unique_ptr<LECBManagerNotifier> notifier = std::make_unique<LECBManagerNotifier>();
    if (role == QLowEnergyController::PeripheralRole) {
        peripheralManager.reset([[DarwinBTPeripheralManager alloc] initWith:notifier.get()],
                                DarwinBluetooth::RetainPolicy::noInitialRetain);
        Q_ASSERT(peripheralManager);
    } else {
        centralManager.reset([[DarwinBTCentralManager alloc] initWith:notifier.get()],
                             DarwinBluetooth::RetainPolicy::noInitialRetain);
        Q_ASSERT(centralManager);
    }

    // FIXME: Q_UNLIKELY
    if (!connectSlots(notifier.get()))
        qCWarning(QT_BT_DARWIN) << "failed to connect to notifier's signal(s)";

    // Ownership was taken by central manager.
    notifier.release();

    return true;
}

void QLowEnergyControllerPrivateDarwin::connectToDevice()
{
    Q_ASSERT_X(state == QLowEnergyController::UnconnectedState,
               Q_FUNC_INFO, "invalid state");

    if (deviceUuid.isNull()) {
        // Wrong constructor was used or invalid UUID was provided.
        return _q_CBManagerError(QLowEnergyController::UnknownRemoteDeviceError);
    }

    if (!lazyInit()) // MissingPermissionsError was emit.
        return;

    // The logic enforcing the role is in the public class.
    Q_ASSERT_X(role != QLowEnergyController::PeripheralRole,
               Q_FUNC_INFO, "invalid role (peripheral)");

    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT_X(leQueue, Q_FUNC_INFO, "invalid LE queue (nullptr)");

    setError(QLowEnergyController::NoError);
    setState(QLowEnergyController::ConnectingState);

    const QBluetoothUuid deviceUuidCopy(deviceUuid);
    DarwinBTCentralManager *manager = centralManager.getAs<DarwinBTCentralManager>();
    dispatch_async(leQueue, ^{
        [manager connectToDevice:deviceUuidCopy];
    });
}

void QLowEnergyControllerPrivateDarwin::disconnectFromDevice()
{
    Q_ASSERT(isValid()); // Check for proper state is in Qt's code.

    if (role == QLowEnergyController::PeripheralRole) {
        // CoreBluetooth API intentionally does not provide any way of closing
        // a connection. All we can do here is to stop the advertisement.
        return stopAdvertising();
    }

    const auto oldState = state;

    if (dispatch_queue_t leQueue = DarwinBluetooth::qt_LE_queue()) {
        setState(QLowEnergyController::ClosingState);
        invalidateServices();

        auto manager = centralManager.getAs<DarwinBTCentralManager>();
        dispatch_async(leQueue, ^{
                           [manager disconnectFromDevice];
                       });

        if (oldState == QLowEnergyController::ConnectingState) {
            // With a pending connect attempt there is no
            // guarantee we'll ever have didDisconnect callback,
            // set the state here and now to make sure we still
            // can connect.
            setState(QLowEnergyController::UnconnectedState);
        }
    } else {
        qCCritical(QT_BT_DARWIN) << "qt LE queue is nil, "
                                    "can not dispatch 'disconnect'";
    }
}

void QLowEnergyControllerPrivateDarwin::discoverServices()
{
    Q_ASSERT_X(state != QLowEnergyController::UnconnectedState,
               Q_FUNC_INFO, "not connected to peripheral");
    Q_ASSERT_X(role != QLowEnergyController::PeripheralRole,
               Q_FUNC_INFO, "invalid role (peripheral)");

    Q_ASSERT(isValid()); // Check we're in a proper state is in q's code.

    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT_X(leQueue, Q_FUNC_INFO, "LE queue not found");

    setState(QLowEnergyController::DiscoveringState);

    DarwinBTCentralManager *manager = centralManager.getAs<DarwinBTCentralManager>();
    dispatch_async(leQueue, ^{
        [manager discoverServices];
    });
}

void QLowEnergyControllerPrivateDarwin::discoverServiceDetails(
        const QBluetoothUuid &serviceUuid, QLowEnergyService::DiscoveryMode mode)
{
    Q_UNUSED(mode);
    if (state != QLowEnergyController::DiscoveredState) {
        qCWarning(QT_BT_DARWIN) << "can not discover service details in the current state, "
                                   "QLowEnergyController::DiscoveredState is expected";
        return;
    }

    if (!serviceList.contains(serviceUuid)) {
        qCWarning(QT_BT_DARWIN) << "unknown service: " << serviceUuid;
        return;
    }

    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT(leQueue);

    ServicePrivate qtService(serviceList.value(serviceUuid));
    qtService->setState(QLowEnergyService::RemoteServiceDiscovering);
    // Copy objects ...
    DarwinBTCentralManager *manager = centralManager.getAs<DarwinBTCentralManager>();
    const QBluetoothUuid serviceUuidCopy(serviceUuid);
    dispatch_async(leQueue, ^{
        [manager discoverServiceDetails:serviceUuidCopy readValues:mode == QLowEnergyService::FullDiscovery];
    });
}

void QLowEnergyControllerPrivateDarwin::requestConnectionUpdate(const QLowEnergyConnectionParameters &params)
{
    Q_UNUSED(params);
    // TODO: implement this, if possible.
    qCWarning(QT_BT_DARWIN) << "Connection update not implemented on your platform";
}

void QLowEnergyControllerPrivateDarwin::addToGenericAttributeList(const QLowEnergyServiceData &service,
                                                                  QLowEnergyHandle startHandle)
{
    // Darwin LE controller implements the addServiceHelper() for adding services, and thus
    // the base class' addServiceHelper(), which calls this function, is not used
    Q_UNUSED(service);
    Q_UNUSED(startHandle);
}

int QLowEnergyControllerPrivateDarwin::mtu() const
{
    // FIXME: check the state - neither public class does,
    // nor us - not fun! E.g. readRssi correctly checked/asserted.

    __block int mtu = DarwinBluetooth::defaultMtu;
    if (!isValid()) // A minimal check.
        return defaultMtu;

    if (const auto leQueue = DarwinBluetooth::qt_LE_queue()) {
        const auto *manager = centralManager.getAs<DarwinBTCentralManager>();
        dispatch_sync(leQueue, ^{
            mtu = [manager mtu];
        });
    }

    return mtu;
}

void QLowEnergyControllerPrivateDarwin::readRssi()
{
    Q_ASSERT(role == QLowEnergyController::CentralRole);
    Q_ASSERT(state == QLowEnergyController::ConnectedState ||
             state == QLowEnergyController::DiscoveringState ||
             state == QLowEnergyController::DiscoveredState);

    if (const auto leQueue = DarwinBluetooth::qt_LE_queue()) {
        const auto *manager = centralManager.getAs<DarwinBTCentralManager>();
        dispatch_async(leQueue, ^{
            [manager readRssi];
        });
    }
}

QLowEnergyService * QLowEnergyControllerPrivateDarwin::addServiceHelper(const QLowEnergyServiceData &service)
{
    if (!lazyInit() || !isValid()) {
        qCWarning(QT_BT_DARWIN) << "invalid peripheral";
        return nullptr;
    }

    for (auto includedService : service.includedServices())
        includedService->d_ptr->type |= QLowEnergyService::IncludedService;

    const auto manager = peripheralManager.getAs<DarwinBTPeripheralManager>();
    Q_ASSERT(manager);
    if (const auto servicePrivate = [manager addService:service]) {
        servicePrivate->setController(this);
        servicePrivate->state = QLowEnergyService::LocalService;
        localServices.insert(servicePrivate->uuid, servicePrivate);
        return new QLowEnergyService(servicePrivate);
    }

    return nullptr;
}

void QLowEnergyControllerPrivateDarwin::_q_connected()
{
    setState(QLowEnergyController::ConnectedState);
    emit q_ptr->connected();
}

void QLowEnergyControllerPrivateDarwin::_q_disconnected()
{
    if (role == QLowEnergyController::CentralRole)
        invalidateServices();

    setState(QLowEnergyController::UnconnectedState);
    emit q_ptr->disconnected();
}

void QLowEnergyControllerPrivateDarwin::_q_mtuChanged(int newValue)
{
    emit q_ptr->mtuChanged(newValue);
}

void QLowEnergyControllerPrivateDarwin::_q_serviceDiscoveryFinished()
{
    Q_ASSERT_X(state == QLowEnergyController::DiscoveringState,
               Q_FUNC_INFO, "invalid state");

    using namespace DarwinBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const services = [centralManager.getAs<DarwinBTCentralManager>() peripheral].services;
    // Now we have to traverse the discovered services tree.
    // Essentially it's an iterative version of more complicated code from the
    // DarwinBTCentralManager's code.
    // All Obj-C entities either auto-release, or guarded by ObjCScopedReferences.
    if (services && [services count]) {
        QMap<QBluetoothUuid, CBService *> discoveredCBServices;
        //1. The first pass - none of this services is 'included' yet (we'll discover 'included'
        //   during the pass 2); we also ignore duplicates (== services with the same UUID)
        // - since we do not have a way to distinguish them later
        //   (our API is using uuids when creating QLowEnergyServices).
        for (CBService *cbService in services) {
            const ServicePrivate newService(qt_createLEService(this, cbService, false));
            if (!newService.data())
                continue;
            if (serviceList.contains(newService->uuid)) {
                // It's a bit stupid we first created it ...
                qCDebug(QT_BT_DARWIN) << "discovered service with a duplicated UUID"
                                   << newService->uuid;
                continue;
            }
            serviceList.insert(newService->uuid, newService);
            discoveredCBServices.insert(newService->uuid, cbService);
        }

        ObjCStrongReference<NSMutableArray> toVisit([[NSMutableArray alloc] initWithArray:services], RetainPolicy::noInitialRetain);
        ObjCStrongReference<NSMutableArray> toVisitNext([[NSMutableArray alloc] init], RetainPolicy::noInitialRetain);
        ObjCStrongReference<NSMutableSet> visited([[NSMutableSet alloc] init], RetainPolicy::noInitialRetain);

        while (true) {
            for (NSUInteger i = 0, e = [toVisit count]; i < e; ++i) {
                CBService *const s = [toVisit objectAtIndex:i];
                if (![visited containsObject:s]) {
                    [visited addObject:s];
                    if (s.includedServices && s.includedServices.count)
                        [toVisitNext addObjectsFromArray:s.includedServices];
                }

                const QBluetoothUuid uuid(qt_uuid(s.UUID));
                if (serviceList.contains(uuid) && discoveredCBServices.value(uuid) == s) {
                    ServicePrivate qtService(serviceList.value(uuid));
                    // Add included UUIDs:
                    qtService->includedServices.append(qt_servicesUuids(s.includedServices));
                }// Else - we ignored this CBService object.
            }

            if (![toVisitNext count])
                break;

            for (NSUInteger i = 0, e = [toVisitNext count]; i < e; ++i) {
                CBService *const s = [toVisitNext objectAtIndex:i];
                const QBluetoothUuid uuid(qt_uuid(s.UUID));
                if (serviceList.contains(uuid)) {
                    if (discoveredCBServices.value(uuid) == s) {
                        ServicePrivate qtService(serviceList.value(uuid));
                        qtService->type |= QLowEnergyService::IncludedService;
                    } // Else this is the duplicate we ignored already.
                } else {
                    // Oh, we do not even have it yet???
                    ServicePrivate newService(qt_createLEService(this, s, true));
                    serviceList.insert(newService->uuid, newService);
                    discoveredCBServices.insert(newService->uuid, s);
                }
            }

            toVisit.swap(toVisitNext);
            toVisitNext.reset([[NSMutableArray alloc] init], RetainPolicy::noInitialRetain);
        }
    } else {
        qCDebug(QT_BT_DARWIN) << "no services found";
    }


    state = QLowEnergyController::DiscoveredState;

    for (auto it = serviceList.constBegin(); it != serviceList.constEnd(); ++it) {
        QMetaObject::invokeMethod(q_ptr, "serviceDiscovered", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothUuid, it.key()));
    }

    QMetaObject::invokeMethod(q_ptr, "stateChanged", Qt::QueuedConnection, Q_ARG(QLowEnergyController::ControllerState, state));
    QMetaObject::invokeMethod(q_ptr, "discoveryFinished", Qt::QueuedConnection);
}

void QLowEnergyControllerPrivateDarwin::_q_serviceDetailsDiscoveryFinished(QSharedPointer<QLowEnergyServicePrivate> service)
{
    QT_BT_MAC_AUTORELEASEPOOL;

    Q_ASSERT(service);

    if (!serviceList.contains(service->uuid)) {
        qCDebug(QT_BT_DARWIN) << "unknown service uuid:"
                              << service->uuid;
        return;
    }

    ServicePrivate qtService(serviceList.value(service->uuid));
    // Assert on handles?
    qtService->startHandle = service->startHandle;
    qtService->endHandle = service->endHandle;
    qtService->characteristicList = service->characteristicList;

    qtService->setState(QLowEnergyService::RemoteServiceDiscovered);
}

void QLowEnergyControllerPrivateDarwin::_q_servicesWereModified()
{
    if (!(state == QLowEnergyController::DiscoveringState
          || state == QLowEnergyController::DiscoveredState)) {
        qCWarning(QT_BT_DARWIN) << "services were modified while controller is not in Discovered/Discovering state";
        return;
    }

    if (state == QLowEnergyController::DiscoveredState)
        invalidateServices();

    setState(QLowEnergyController::ConnectedState);
    q_ptr->discoverServices();
}

void QLowEnergyControllerPrivateDarwin::_q_characteristicRead(QLowEnergyHandle charHandle,
                                                              const QByteArray &value)
{
    Q_ASSERT_X(charHandle, Q_FUNC_INFO, "invalid characteristic handle(0)");

    ServicePrivate service(serviceForHandle(charHandle));
    if (service.isNull())
        return;

    QLowEnergyCharacteristic characteristic(characteristicForHandle(charHandle));
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_DARWIN) << "unknown characteristic";
        return;
    }

    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(charHandle, value, false);

    emit service->characteristicRead(characteristic, value);
}

void QLowEnergyControllerPrivateDarwin::_q_characteristicWritten(QLowEnergyHandle charHandle,
                                                                 const QByteArray &value)
{
    Q_ASSERT_X(charHandle, Q_FUNC_INFO, "invalid characteristic handle(0)");

    ServicePrivate service(serviceForHandle(charHandle));
    if (service.isNull()) {
        qCWarning(QT_BT_DARWIN) << "can not find service for characteristic handle"
                                << charHandle;
        return;
    }

    QLowEnergyCharacteristic characteristic(characteristicForHandle(charHandle));
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_DARWIN) << "unknown characteristic";
        return;
    }

    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(charHandle, value, false);

    emit service->characteristicWritten(characteristic, value);
}

void QLowEnergyControllerPrivateDarwin::_q_characteristicUpdated(QLowEnergyHandle charHandle,
                                                                 const QByteArray &value)
{
    // TODO: write/update notifications are quite similar (except asserts/warnings messages
    // and different signals emitted). Merge them into one function?
    Q_ASSERT_X(charHandle, Q_FUNC_INFO, "invalid characteristic handle(0)");

    ServicePrivate service(serviceForHandle(charHandle));
    if (service.isNull()) {
        // This can be an error (no characteristic found for this handle),
        // it can also be that we set notify value before the service
        // was reported (serviceDetailsDiscoveryFinished) - this happens,
        // if we read a descriptor (characteristic client configuration),
        // and it's (pre)set.
        return;
    }

    QLowEnergyCharacteristic characteristic(characteristicForHandle(charHandle));
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_DARWIN) << "unknown characteristic";
        return;
    }

    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(charHandle, value, false);

    emit service->characteristicChanged(characteristic, value);
}

void QLowEnergyControllerPrivateDarwin::_q_descriptorRead(QLowEnergyHandle dHandle,
                                                          const QByteArray &value)
{
    Q_ASSERT_X(dHandle, Q_FUNC_INFO, "invalid descriptor handle (0)");

    const QLowEnergyDescriptor qtDescriptor(descriptorForHandle(dHandle));
    if (!qtDescriptor.isValid()) {
        qCWarning(QT_BT_DARWIN) << "unknown descriptor" << dHandle;
        return;
    }

    ServicePrivate service(serviceForHandle(qtDescriptor.characteristicHandle()));
    updateValueOfDescriptor(qtDescriptor.characteristicHandle(), dHandle, value, false);
    emit service->descriptorRead(qtDescriptor, value);
}

void QLowEnergyControllerPrivateDarwin::_q_descriptorWritten(QLowEnergyHandle dHandle,
                                                             const QByteArray &value)
{
    Q_ASSERT_X(dHandle, Q_FUNC_INFO, "invalid descriptor handle (0)");

    const QLowEnergyDescriptor qtDescriptor(descriptorForHandle(dHandle));
    if (!qtDescriptor.isValid()) {
        qCWarning(QT_BT_DARWIN) << "unknown descriptor" << dHandle;
        return;
    }

    ServicePrivate service(serviceForHandle(qtDescriptor.characteristicHandle()));
    // TODO: test if this data is what we expected.
    updateValueOfDescriptor(qtDescriptor.characteristicHandle(), dHandle, value, false);
    emit service->descriptorWritten(qtDescriptor, value);
}

void QLowEnergyControllerPrivateDarwin::_q_notificationEnabled(QLowEnergyHandle charHandle,
                                                               bool enabled)
{
    // CoreBluetooth in peripheral role does not allow mutable descriptors,
    // in central we can only call setNotification:enabled/disabled.
    // But from Qt API's point of view, a central has to write into
    // client characteristic configuration descriptor. So here we emulate
    // such a write (we cannot say if it's a notification or indication and
    // report as both).

    Q_ASSERT_X(role == QLowEnergyController::PeripheralRole, Q_FUNC_INFO,
               "controller has an invalid role, 'peripheral' expected");
    Q_ASSERT_X(charHandle, Q_FUNC_INFO, "invalid characteristic handle (0)");

    const QLowEnergyCharacteristic qtChar(characteristicForHandle(charHandle));
    if (!qtChar.isValid()) {
        qCWarning(QT_BT_DARWIN) << "unknown characteristic" << charHandle;
        return;
    }

    const QLowEnergyDescriptor qtDescriptor =
        qtChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (!qtDescriptor.isValid()) {
        qCWarning(QT_BT_DARWIN) << "characteristic" << charHandle
                                << "does not have a client characteristic "
                                   "descriptor";
        return;
    }

    ServicePrivate service(serviceForHandle(charHandle));
    if (service.data()) {
        // It's a 16-bit value, the least significant bit is for notifications,
        // the next one - for indications (thus 1 means notifications enabled,
        // 2 - indications enabled).
        // 3 is the maximum value and it means both enabled.
        QByteArray value(2, 0);
        if (enabled)
            value[0] = 3;
        updateValueOfDescriptor(charHandle, qtDescriptor.handle(), value, false);
        emit service->descriptorWritten(qtDescriptor, value);
    }
}

void QLowEnergyControllerPrivateDarwin::_q_LEnotSupported()
{
    // Report as an error. But this should not be possible
    // actually: before connecting to any device, we have
    // to discover it, if it was discovered ... LE _must_
    // be supported.
}

void QLowEnergyControllerPrivateDarwin::_q_CBManagerError(QLowEnergyController::Error errorCode)
{
    qCDebug(QT_BT_DARWIN) << "QLowEnergyController error:" << errorCode << "in state:" << state;
    // This function handles errors reported while connecting to a remote device
    // and also other errors in general.
    setError(errorCode);

    if (state == QLowEnergyController::ConnectingState
            || state == QLowEnergyController::AdvertisingState) {
        setState(QLowEnergyController::UnconnectedState);
    } else if (state == QLowEnergyController::DiscoveringState) {
        // An error occurred during service discovery, finish the discovery.
        setState(QLowEnergyController::ConnectedState);
        emit q_ptr->discoveryFinished();
    }

    // In any other case we stay in Discovered, it's
    // a service/characteristic - related error.
}

void QLowEnergyControllerPrivateDarwin::_q_CBManagerError(const QBluetoothUuid &serviceUuid,
                                                          QLowEnergyController::Error errorCode)
{
    // Errors reported while discovering service details etc.
    Q_UNUSED(errorCode); // TODO: setError?

    // We failed to discover any characteristics/descriptors.
    if (serviceList.contains(serviceUuid)) {
        ServicePrivate qtService(serviceList.value(serviceUuid));
        qtService->setState(QLowEnergyService::InvalidService);
    } else {
        qCDebug(QT_BT_DARWIN) << "error reported for unknown service"
                              << serviceUuid;
    }
}

void QLowEnergyControllerPrivateDarwin::_q_CBManagerError(const QBluetoothUuid &serviceUuid,
                                                          QLowEnergyService::ServiceError errorCode)
{
    if (!serviceList.contains(serviceUuid)) {
        qCDebug(QT_BT_DARWIN) << "unknown service uuid:"
                              << serviceUuid;
        return;
    }

    ServicePrivate service(serviceList.value(serviceUuid));
    service->setError(errorCode);
}

void QLowEnergyControllerPrivateDarwin::setNotifyValue(QSharedPointer<QLowEnergyServicePrivate> service,
                                                       QLowEnergyHandle charHandle,
                                                       const QByteArray &newValue)
{
    Q_ASSERT_X(!service.isNull(), Q_FUNC_INFO, "invalid service (null)");

    if (role == QLowEnergyController::PeripheralRole) {
        qCWarning(QT_BT_DARWIN) << "invalid role (peripheral)";
        service->setError(QLowEnergyService::DescriptorWriteError);
        return;
    }

    if (newValue.size() > 2) {
        // Qt's API requires an error on such write.
        // With Core Bluetooth we do not write any descriptor,
        // but instead call a special method. So it's better to
        // intercept wrong data size here:
        qCWarning(QT_BT_DARWIN) << "client characteristic configuration descriptor"
                                   "is 2 bytes, but value size is: " << newValue.size();
        service->setError(QLowEnergyService::DescriptorWriteError);
        return;
    }

    if (!serviceList.contains(service->uuid)) {
        qCWarning(QT_BT_DARWIN) << "no service with uuid:" << service->uuid << "found";
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_DARWIN) << "no characteristic with handle:"
                              << charHandle << "found";
        return;
    }

    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT_X(leQueue, Q_FUNC_INFO, "no LE queue found");

    DarwinBTCentralManager *manager = centralManager.getAs<DarwinBTCentralManager>();
    const QBluetoothUuid serviceUuid(service->uuid);
    const QByteArray newValueCopy(newValue);
    dispatch_async(leQueue, ^{
        [manager setNotifyValue:newValueCopy
                 forCharacteristic:charHandle
                 onService:serviceUuid];
    });
}

void QLowEnergyControllerPrivateDarwin::readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                                                           const QLowEnergyHandle charHandle)
{
    Q_ASSERT_X(!service.isNull(), Q_FUNC_INFO, "invalid service (null)");

    if (role == QLowEnergyController::PeripheralRole) {
        qCWarning(QT_BT_DARWIN) << "invalid role (peripheral)";
        return;
    }

    if (!serviceList.contains(service->uuid)) {
        qCWarning(QT_BT_DARWIN) << "no service with uuid:"
                                << service->uuid << "found";
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_DARWIN) << "no characteristic with handle:"
                              << charHandle << "found";
        return;
    }

    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT_X(leQueue, Q_FUNC_INFO, "no LE queue found");

    // Attention! We have to copy UUID.
    DarwinBTCentralManager *manager = centralManager.getAs<DarwinBTCentralManager>();
    const QBluetoothUuid serviceUuid(service->uuid);
    dispatch_async(leQueue, ^{
        [manager readCharacteristic:charHandle onService:serviceUuid];
    });
}

void QLowEnergyControllerPrivateDarwin::writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                                                            const QLowEnergyHandle charHandle, const QByteArray &newValue,
                                                            QLowEnergyService::WriteMode mode)
{
    Q_ASSERT_X(!service.isNull(), Q_FUNC_INFO, "invalid service (null)");

    // We can work only with services found on a given peripheral
    // (== created by the given LE controller).

    if (!serviceList.contains(service->uuid) && !localServices.contains(service->uuid)) {
        qCWarning(QT_BT_DARWIN) << "no service with uuid:"
                                << service->uuid << " found";
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_DARWIN) << "no characteristic with handle:"
                              << charHandle << " found";
        return;
    }

    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT_X(leQueue, Q_FUNC_INFO, "no LE queue found");
    // Attention! We have to copy objects!
    const QByteArray newValueCopy(newValue);
    if (role == QLowEnergyController::CentralRole) {
        const QBluetoothUuid serviceUuid(service->uuid);
        const auto manager = centralManager.getAs<DarwinBTCentralManager>();
        dispatch_async(leQueue, ^{
            [manager write:newValueCopy
                charHandle:charHandle
                 onService:serviceUuid
                 withResponse:mode == QLowEnergyService::WriteWithResponse];
        });
    } else {
        const auto manager = peripheralManager.getAs<DarwinBTPeripheralManager>();
        dispatch_async(leQueue, ^{
            [manager write:newValueCopy charHandle:charHandle];
        });
    }
}

quint16 QLowEnergyControllerPrivateDarwin::updateValueOfCharacteristic(QLowEnergyHandle charHandle,
                                                                       const QByteArray &value,
                                                                       bool appendValue)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
    if (!service.isNull()) {
        CharacteristicDataMap::iterator charIt = service->characteristicList.find(charHandle);
        if (charIt != service->characteristicList.end()) {
            QLowEnergyServicePrivate::CharData &charData = charIt.value();
            if (appendValue)
                charData.value += value;
            else
                charData.value = value;

            return charData.value.size();
        }
    }

    return 0;
}

void QLowEnergyControllerPrivateDarwin::readDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                                                       const QLowEnergyHandle charHandle,
                                                       const QLowEnergyHandle descriptorHandle)
{
    Q_UNUSED(charHandle); // Hehe, yes!

    Q_ASSERT_X(!service.isNull(), Q_FUNC_INFO, "invalid service (null)");

    if (role == QLowEnergyController::PeripheralRole) {
        qCWarning(QT_BT_DARWIN) << "invalid role (peripheral)";
        return;
    }

    if (!serviceList.contains(service->uuid)) {
        qCWarning(QT_BT_DARWIN) << "no service with uuid:"
                                << service->uuid << "found";
        return;
    }

    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    if (!leQueue) {
        qCWarning(QT_BT_DARWIN) << "no LE queue found";
        return;
    }
    // Attention! Copy objects!
    const QBluetoothUuid serviceUuid(service->uuid);
    DarwinBTCentralManager * const manager = centralManager.getAs<DarwinBTCentralManager>();
    dispatch_async(leQueue, ^{
        [manager readDescriptor:descriptorHandle
                 onService:serviceUuid];
    });
}

void QLowEnergyControllerPrivateDarwin::writeDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                                                        const QLowEnergyHandle charHandle,
                                                        const QLowEnergyHandle descriptorHandle,
                                                        const QByteArray &newValue)
{
    Q_UNUSED(charHandle);

    Q_ASSERT_X(!service.isNull(), Q_FUNC_INFO, "invalid service (null)");

    if (role == QLowEnergyController::PeripheralRole) {
        qCWarning(QT_BT_DARWIN) << "invalid role (peripheral)";
        return;
    }

    // We can work only with services found on a given peripheral
    // (== created by the given LE controller),
    // otherwise we can not write anything at all.
    if (!serviceList.contains(service->uuid)) {
        qCWarning(QT_BT_DARWIN) << "no service with uuid:"
                                << service->uuid << " found";
        return;
    }

    dispatch_queue_t leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT_X(leQueue, Q_FUNC_INFO, "no LE queue found");
    // Attention! Copy objects!
    const QBluetoothUuid serviceUuid(service->uuid);
    DarwinBTCentralManager * const manager = centralManager.getAs<DarwinBTCentralManager>();
    const QByteArray newValueCopy(newValue);
    dispatch_async(leQueue, ^{
        [manager write:newValueCopy
                 descHandle:descriptorHandle
                 onService:serviceUuid];
    });
}

quint16 QLowEnergyControllerPrivateDarwin::updateValueOfDescriptor(QLowEnergyHandle charHandle, QLowEnergyHandle descHandle,
                                                                   const QByteArray &value, bool appendValue)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
    if (!service.isNull()) {
        CharacteristicDataMap::iterator charIt = service->characteristicList.find(charHandle);
        if (charIt != service->characteristicList.end()) {
            QLowEnergyServicePrivate::CharData &charData = charIt.value();

            DescriptorDataMap::iterator descIt = charData.descriptorList.find(descHandle);
            if (descIt != charData.descriptorList.end()) {
                QLowEnergyServicePrivate::DescData &descDetails = descIt.value();

                if (appendValue)
                    descDetails.value += value;
                else
                    descDetails.value = value;

                return descDetails.value.size();
            }
        }
    }

    return 0;
}

bool QLowEnergyControllerPrivateDarwin::connectSlots(DarwinBluetooth::LECBManagerNotifier *notifier)
{
    using DarwinBluetooth::LECBManagerNotifier;

    Q_ASSERT_X(notifier, Q_FUNC_INFO, "invalid notifier object (null)");

    bool ok = connect(notifier, &LECBManagerNotifier::connected,
                      this, &QLowEnergyControllerPrivateDarwin::_q_connected);
    ok = ok && connect(notifier, &LECBManagerNotifier::disconnected,
                       this, &QLowEnergyControllerPrivateDarwin::_q_disconnected);
    ok = ok && connect(notifier, &LECBManagerNotifier::serviceDiscoveryFinished,
                       this, &QLowEnergyControllerPrivateDarwin::_q_serviceDiscoveryFinished);
    ok = ok && connect(notifier, &LECBManagerNotifier::servicesWereModified,
                       this, &QLowEnergyControllerPrivateDarwin::_q_servicesWereModified);
    ok = ok && connect(notifier, &LECBManagerNotifier::serviceDetailsDiscoveryFinished,
                       this, &QLowEnergyControllerPrivateDarwin::_q_serviceDetailsDiscoveryFinished);
    ok = ok && connect(notifier, &LECBManagerNotifier::characteristicRead,
                       this, &QLowEnergyControllerPrivateDarwin::_q_characteristicRead);
    ok = ok && connect(notifier, &LECBManagerNotifier::characteristicWritten,
                       this, &QLowEnergyControllerPrivateDarwin::_q_characteristicWritten);
    ok = ok && connect(notifier, &LECBManagerNotifier::characteristicUpdated,
                       this, &QLowEnergyControllerPrivateDarwin::_q_characteristicUpdated);
    ok = ok && connect(notifier, &LECBManagerNotifier::descriptorRead,
                       this, &QLowEnergyControllerPrivateDarwin::_q_descriptorRead);
    ok = ok && connect(notifier, &LECBManagerNotifier::descriptorWritten,
                       this, &QLowEnergyControllerPrivateDarwin::_q_descriptorWritten);
    ok = ok && connect(notifier, &LECBManagerNotifier::notificationEnabled,
                       this, &QLowEnergyControllerPrivateDarwin::_q_notificationEnabled);
    ok = ok && connect(notifier, &LECBManagerNotifier::LEnotSupported,
                       this, &QLowEnergyControllerPrivateDarwin::_q_LEnotSupported);
    ok = ok && connect(notifier, SIGNAL(CBManagerError(QLowEnergyController::Error)),
                       this, SLOT(_q_CBManagerError(QLowEnergyController::Error)));
    ok = ok && connect(notifier, SIGNAL(CBManagerError(QBluetoothUuid,QLowEnergyController::Error)),
                       this, SLOT(_q_CBManagerError(QBluetoothUuid,QLowEnergyController::Error)));
    ok = ok && connect(notifier, SIGNAL(CBManagerError(QBluetoothUuid,QLowEnergyService::ServiceError)),
                       this, SLOT(_q_CBManagerError(QBluetoothUuid,QLowEnergyService::ServiceError)));
    ok = ok && connect(notifier, &LECBManagerNotifier::mtuChanged, this,
                       &QLowEnergyControllerPrivateDarwin::_q_mtuChanged);
    ok = ok && connect(notifier, &LECBManagerNotifier::rssiUpdated, q_ptr,
                       &QLowEnergyController::rssiRead, Qt::QueuedConnection);

    if (!ok)
        notifier->disconnect();

    return ok;
}

void QLowEnergyControllerPrivateDarwin::startAdvertising(const QLowEnergyAdvertisingParameters &params,
                                                         const QLowEnergyAdvertisingData &advertisingData,
                                                         const QLowEnergyAdvertisingData &scanResponseData)
{
    if (!lazyInit()) // Error was emit already.
        return;

    auto leQueue(DarwinBluetooth::qt_LE_queue());
    Q_ASSERT_X(leQueue, Q_FUNC_INFO, "invalid LE queue (nullptr)");

    const auto manager = peripheralManager.getAs<DarwinBTPeripheralManager>();
    [manager setParameters:params data:advertisingData scanResponse:scanResponseData];

    setState(QLowEnergyController::AdvertisingState);

    dispatch_async(leQueue, ^{
        [manager startAdvertising];
    });
}

void QLowEnergyControllerPrivateDarwin::stopAdvertising()
{
    if (!isValid())
        return _q_CBManagerError(QLowEnergyController::UnknownError);

    if (state != QLowEnergyController::AdvertisingState) {
        qCDebug(QT_BT_DARWIN) << "cannot stop advertising, called in state" << state;
        return;
    }

    const auto leQueue = DarwinBluetooth::qt_LE_queue();
    Q_ASSERT_X(leQueue, Q_FUNC_INFO, "invalid LE queue (nullptr)");
    const auto manager = peripheralManager.getAs<DarwinBTPeripheralManager>();
    dispatch_sync(leQueue, ^{
                      [manager stopAdvertising];
                  });

    setState(QLowEnergyController::UnconnectedState);
}

QT_END_NAMESPACE

