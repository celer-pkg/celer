// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergycontroller_android_p.h"
#include "android/androidutils_p.h"
#include "android/jni_android_p.h"
#include <QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtCore/q26numeric.h>
#include <QtBluetooth/QLowEnergyServiceData>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/QLowEnergyAdvertisingData>
#include <QtBluetooth/QLowEnergyAdvertisingParameters>
#include <QtBluetooth/QLowEnergyConnectionParameters>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

// BT Core v5.3, 3.2.9, Vol 3, Part F
const int BTLE_MAX_ATTRIBUTE_VALUE_SIZE = 512;

// Conversion: QBluetoothUuid -> java.util.UUID
static QJniObject javaUuidfromQtUuid(const QBluetoothUuid& uuid)
{
    // cut off leading and trailing brackets
    const QString output = uuid.toString(QUuid::WithoutBraces);

    QJniObject javaString = QJniObject::fromString(output);
    QJniObject javaUuid = QJniObject::callStaticMethod<QtJniTypes::UUID>(
                QtJniTypes::Traits<QtJniTypes::UUID>::className(), "fromString",
                javaString.object<jstring>());

    return javaUuid;
}

QLowEnergyControllerPrivateAndroid::QLowEnergyControllerPrivateAndroid()
    : QLowEnergyControllerPrivate(),
      hub(0)
{
    registerQLowEnergyControllerMetaType();
}

QLowEnergyControllerPrivateAndroid::~QLowEnergyControllerPrivateAndroid()
{
    if (role == QLowEnergyController::PeripheralRole) {
        if (hub)
            hub->javaObject().callMethod<void>("disconnectServer");
    }
}

void QLowEnergyControllerPrivateAndroid::init()
{
    const bool isPeripheral = (role == QLowEnergyController::PeripheralRole);

    if (isPeripheral) {
        qRegisterMetaType<QJniObject>();
        hub = new LowEnergyNotificationHub(remoteDevice, isPeripheral, this);
        // we only connect to the peripheral role specific signals
        // TODO add connections as they get added later on
        connect(hub, &LowEnergyNotificationHub::connectionUpdated,
                this, &QLowEnergyControllerPrivateAndroid::connectionUpdated);
        connect(hub, &LowEnergyNotificationHub::mtuChanged,
                this, &QLowEnergyControllerPrivateAndroid::mtuChanged);
        connect(hub, &LowEnergyNotificationHub::advertisementError,
                this, &QLowEnergyControllerPrivateAndroid::advertisementError);
        connect(hub, &LowEnergyNotificationHub::serverCharacteristicChanged,
                this, &QLowEnergyControllerPrivateAndroid::serverCharacteristicChanged);
        connect(hub, &LowEnergyNotificationHub::serverDescriptorWritten,
                this, &QLowEnergyControllerPrivateAndroid::serverDescriptorWritten);
    } else {
        hub = new LowEnergyNotificationHub(remoteDevice, isPeripheral, this);
        // we only connect to the central role specific signals
        connect(hub, &LowEnergyNotificationHub::connectionUpdated,
                this, &QLowEnergyControllerPrivateAndroid::connectionUpdated);
        connect(hub, &LowEnergyNotificationHub::mtuChanged,
                this, &QLowEnergyControllerPrivateAndroid::mtuChanged);
        connect(hub, &LowEnergyNotificationHub::servicesDiscovered,
                this, &QLowEnergyControllerPrivateAndroid::servicesDiscovered);
        connect(hub, &LowEnergyNotificationHub::serviceDetailsDiscoveryFinished,
                this, &QLowEnergyControllerPrivateAndroid::serviceDetailsDiscoveryFinished);
        connect(hub, &LowEnergyNotificationHub::characteristicRead,
                this, &QLowEnergyControllerPrivateAndroid::characteristicRead);
        connect(hub, &LowEnergyNotificationHub::descriptorRead,
                this, &QLowEnergyControllerPrivateAndroid::descriptorRead);
        connect(hub, &LowEnergyNotificationHub::characteristicWritten,
                this, &QLowEnergyControllerPrivateAndroid::characteristicWritten);
        connect(hub, &LowEnergyNotificationHub::descriptorWritten,
                this, &QLowEnergyControllerPrivateAndroid::descriptorWritten);
        connect(hub, &LowEnergyNotificationHub::characteristicChanged,
                this, &QLowEnergyControllerPrivateAndroid::characteristicChanged);
        connect(hub, &LowEnergyNotificationHub::serviceError,
                this, &QLowEnergyControllerPrivateAndroid::serviceError);
        connect(hub, &LowEnergyNotificationHub::remoteRssiRead,
                this, &QLowEnergyControllerPrivateAndroid::remoteRssiRead);
    }
}

void QLowEnergyControllerPrivateAndroid::connectToDevice()
{
    if (!hub) {
        qCCritical(QT_BT_ANDROID) << "connectToDevice() LE controller has not been initialized";
        return;
    }

    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        // This is unlikely to happen as a valid local adapter is a precondition
        setError(QLowEnergyController::MissingPermissionsError);
        qCWarning(QT_BT_ANDROID) << "connectToDevice() failed due to missing permissions";
        return;
    }

    // required to pass unit test on default backend
    if (remoteDevice.isNull()) {
        qCWarning(QT_BT_ANDROID) << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    setState(QLowEnergyController::ConnectingState);

    if (!hub->javaObject().isValid()) {
        qCWarning(QT_BT_ANDROID) << "Cannot initiate QtBluetoothLE";
        setError(QLowEnergyController::ConnectionError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    bool result = hub->javaObject().callMethod<jboolean>("connect");
    if (!result) {
        setError(QLowEnergyController::ConnectionError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }
}

void QLowEnergyControllerPrivateAndroid::disconnectFromDevice()
{
    /* Catch an Android timeout bug. If the device is connecting but cannot
     * physically connect it seems to ignore the disconnect call below.
     * At least BluetoothGattCallback.onConnectionStateChange never
     * arrives. The next BluetoothGatt.connect() works just fine though.
     * */

    QLowEnergyController::ControllerState oldState = state;
    setState(QLowEnergyController::ClosingState);

    if (hub) {
        if (role == QLowEnergyController::PeripheralRole)
            hub->javaObject().callMethod<void>("disconnectServer");
        else
            hub->javaObject().callMethod<void>("disconnect");
    }

    if (oldState == QLowEnergyController::ConnectingState)
        setState(QLowEnergyController::UnconnectedState);
}

void QLowEnergyControllerPrivateAndroid::discoverServices()
{
    // No need to check bluetooth permissions here as 'connected' is a precondition

    if (hub && hub->javaObject().callMethod<jboolean>("discoverServices")) {
        qCDebug(QT_BT_ANDROID) << "Service discovery initiated";
    } else {
        //revert to connected state
        setError(QLowEnergyController::NetworkError);
        setState(QLowEnergyController::ConnectedState);
    }
}

void QLowEnergyControllerPrivateAndroid::discoverServiceDetails(
        const QBluetoothUuid &service, QLowEnergyService::DiscoveryMode mode)
{
    Q_UNUSED(mode);
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_ANDROID) << "Discovery of unknown service" << service.toString()
                                 << "not possible";
        return;
    }

    if (!hub)
        return;

    QString tempUuid = service.toString(QUuid::WithoutBraces);

    QJniEnvironment env;
    QJniObject uuid = QJniObject::fromString(tempUuid);
    bool readAllValues = mode == QLowEnergyService::FullDiscovery;
    bool result = hub->javaObject().callMethod<jboolean>("discoverServiceDetails",
                                                         uuid.object<jstring>(),
                                                         readAllValues);
    if (!result) {
        QSharedPointer<QLowEnergyServicePrivate> servicePrivate =
                serviceList.value(service);
        if (!servicePrivate.isNull()) {
            servicePrivate->setError(QLowEnergyService::UnknownError);
            servicePrivate->setState(QLowEnergyService::RemoteService);
        }
        qCWarning(QT_BT_ANDROID) << "Cannot discover details for" << service.toString();
        return;
    }

    qCDebug(QT_BT_ANDROID) << "Discovery of" << service << "started";
}

void QLowEnergyControllerPrivateAndroid::writeCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QByteArray &newValue,
        QLowEnergyService::WriteMode mode)
{
    //TODO don't ignore WriteWithResponse, right now we assume responses
    Q_ASSERT(!service.isNull());

    if (!service->characteristicList.contains(charHandle))
        return;

    QJniEnvironment env;
    const jsize nativeSize = q26::saturate_cast<jsize>(newValue.size());
    jbyteArray payload;
    payload = env->NewByteArray(nativeSize);
    env->SetByteArrayRegion(payload, 0, nativeSize,
                            (jbyte *)newValue.constData());

    bool result = false;
    if (hub) {
        if (role == QLowEnergyController::CentralRole) {
            qCDebug(QT_BT_ANDROID) << "Write characteristic with handle " << charHandle
                     << newValue.toHex() << "(service:" << service->uuid
                     << ", writeWithResponse:" << (mode == QLowEnergyService::WriteWithResponse)
                     << ", signed:" << (mode == QLowEnergyService::WriteSigned) << ")";
            result = hub->javaObject().callMethod<jboolean>("writeCharacteristic",
                                                            charHandle, payload, jint(mode));
        } else { // peripheral mode
            qCDebug(QT_BT_ANDROID) << "Write server characteristic with handle " << charHandle
                     << newValue.toHex() << "(service:" << service->uuid;

            const auto &characteristic = characteristicForHandle(charHandle);
            if (characteristic.isValid()) {
                const QJniObject charUuid = javaUuidfromQtUuid(characteristic.uuid());
                result = hub->javaObject().callMethod<jboolean>(
                            "writeCharacteristic",
                            service->androidService.object<QtJniTypes::BluetoothGattService>(),
                            charUuid.object<QtJniTypes::UUID>(), payload);
                if (result)
                    service->characteristicList[charHandle].value = newValue;
            }
        }
    }

    env->DeleteLocalRef(payload);

    if (!result)
        service->setError(QLowEnergyService::CharacteristicWriteError);
}

void QLowEnergyControllerPrivateAndroid::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle,
        const QByteArray &newValue)
{
    Q_ASSERT(!service.isNull());

    QJniEnvironment env;
    const jsize nativeSize = q26::saturate_cast<jsize>(newValue.size());
    jbyteArray payload;
    payload = env->NewByteArray(nativeSize);
    env->SetByteArrayRegion(payload, 0, nativeSize,
                            (jbyte *)newValue.constData());

    bool result = false;
    if (hub) {
        if (role == QLowEnergyController::CentralRole) {
            qCDebug(QT_BT_ANDROID) << "Write descriptor with handle " << descHandle
                     << newValue.toHex() << "(service:" << service->uuid << ")";
            result = hub->javaObject().callMethod<jboolean>("writeDescriptor",
                                                            descHandle, payload);
        } else {
            const auto &characteristic = characteristicForHandle(charHandle);
            const auto &descriptor = descriptorForHandle(descHandle);
            if (characteristic.isValid() && descriptor.isValid()) {
                qCDebug(QT_BT_ANDROID) << "Write descriptor" << descriptor.uuid()
                                   << "(service:" << service->uuid
                                   << "char: " << characteristic.uuid() << ")";
                const QJniObject charUuid = javaUuidfromQtUuid(characteristic.uuid());
                const QJniObject descUuid = javaUuidfromQtUuid(descriptor.uuid());
                result = hub->javaObject().callMethod<jboolean>(
                          "writeDescriptor",
                          service->androidService.object<QtJniTypes::BluetoothGattService>(),
                          charUuid.object<QtJniTypes::UUID>(), descUuid.object<QtJniTypes::UUID>(),
                          payload);
                if (result)
                    service->characteristicList[charHandle].descriptorList[descHandle].value = newValue;
            }
        }
    }

    env->DeleteLocalRef(payload);

    if (!result)
        service->setError(QLowEnergyService::DescriptorWriteError);
}

void QLowEnergyControllerPrivateAndroid::readCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle)
{
    Q_ASSERT(!service.isNull());

    if (!service->characteristicList.contains(charHandle))
        return;

    QJniEnvironment env;
    bool result = false;
    if (hub) {
        qCDebug(QT_BT_ANDROID) << "Read characteristic with handle"
                               <<  charHandle << service->uuid;
        result = hub->javaObject().callMethod<jboolean>("readCharacteristic", charHandle);
    }

    if (!result)
        service->setError(QLowEnergyService::CharacteristicReadError);
}

void QLowEnergyControllerPrivateAndroid::readDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle /*charHandle*/,
        const QLowEnergyHandle descriptorHandle)
{
    Q_ASSERT(!service.isNull());

    QJniEnvironment env;
    bool result = false;
    if (hub) {
        qCDebug(QT_BT_ANDROID) << "Read descriptor with handle"
                               <<  descriptorHandle << service->uuid;
        result = hub->javaObject().callMethod<jboolean>("readDescriptor", descriptorHandle);
    }

    if (!result)
        service->setError(QLowEnergyService::DescriptorReadError);
}

void QLowEnergyControllerPrivateAndroid::connectionUpdated(
        QLowEnergyController::ControllerState newState,
        QLowEnergyController::Error errorCode)
{
    qCDebug(QT_BT_ANDROID) << "Connection updated:"
                           << "error:" << errorCode
                           << "oldState:" << state
                           << "newState:" << newState;

    if (role == QLowEnergyController::PeripheralRole)
        peripheralConnectionUpdated(newState, errorCode);
    else
        centralConnectionUpdated(newState, errorCode);
}

void QLowEnergyControllerPrivateAndroid::mtuChanged(int mtu)
{
    Q_Q(QLowEnergyController);
    qCDebug(QT_BT_ANDROID) << "MTU updated:"
                           << "mtu:" << mtu;
    emit q->mtuChanged(mtu);
}

void QLowEnergyControllerPrivateAndroid::remoteRssiRead(int rssi, bool success)
{
    Q_Q(QLowEnergyController);
    if (success) {
        // BT Core v5.3, 7.5.4, Vol 4, Part E
        // The LE RSSI can take values -127..127 => narrowing to qint16 is safe
        emit q->rssiRead(rssi);
    } else {
        qCDebug(QT_BT_ANDROID) << "Reading remote RSSI failed";
        setError(QLowEnergyController::RssiReadError);
    }
}

// called if server/peripheral
void QLowEnergyControllerPrivateAndroid::peripheralConnectionUpdated(
        QLowEnergyController::ControllerState newState,
        QLowEnergyController::Error errorCode)
{
    // Java errorCode can be larger than max QLowEnergyController::Error
    if (errorCode > QLowEnergyController::AdvertisingError)
        errorCode = QLowEnergyController::UnknownError;

    if (errorCode != QLowEnergyController::NoError)
        setError(errorCode);

    const QLowEnergyController::ControllerState oldState = state;
    setState(newState);

    // disconnect implies stop of advertisement
    if (newState == QLowEnergyController::UnconnectedState)
        stopAdvertising();

    // The remote name and address may have changed
    if (hub) {
        remoteDevice = QBluetoothAddress(
                    hub->javaObject().callMethod<jstring>("remoteAddress").toString());
        remoteName = hub->javaObject().callMethod<jstring>("remoteName").toString();
    }

    Q_Q(QLowEnergyController);
    // Emit (dis)connected if the connection state changes
    if (oldState == QLowEnergyController::ConnectedState
            && newState != QLowEnergyController::ConnectedState) {
        emit q->disconnected();
    } else if (newState == QLowEnergyController::ConnectedState
                 && oldState != QLowEnergyController::ConnectedState) {
        emit q->connected();
    }
}

// called if client/central
void QLowEnergyControllerPrivateAndroid::centralConnectionUpdated(
        QLowEnergyController::ControllerState newState,
        QLowEnergyController::Error errorCode)
{
    Q_Q(QLowEnergyController);

    const QLowEnergyController::ControllerState oldState = state;

    if (errorCode != QLowEnergyController::NoError) {
        // ConnectionError if transition from Connecting to Connected
        if (oldState == QLowEnergyController::ConnectingState) {
            setError(QLowEnergyController::ConnectionError);
            /* There is a bug in Android, when connecting to an unconnectable
             * device. The connection times out and Android sends error code
             * 133 (doesn't exist) and STATE_CONNECTED. A subsequent disconnect()
             * call never sends a STATE_DISCONNECTED either.
             * As workaround we will trigger disconnect when we encounter
             * error during connect attempt. This leaves the controller
             * in a cleaner state.
             * */
            newState = QLowEnergyController::UnconnectedState;
        }
        else
            setError(errorCode);
    }

    setState(newState);
    if (newState == QLowEnergyController::UnconnectedState
            && !(oldState == QLowEnergyController::UnconnectedState
                || oldState == QLowEnergyController::ConnectingState)) {

        // Invalidate the services if the disconnect came from the remote end.
        // Qtherwise we disconnected via QLowEnergyController::disconnectDevice() which
        // triggered invalidation already
        if (!serviceList.isEmpty()) {
            Q_ASSERT(oldState != QLowEnergyController::ClosingState);
            invalidateServices();
        }
        emit q->disconnected();
    } else if (newState == QLowEnergyController::ConnectedState
               && oldState != QLowEnergyController::ConnectedState ) {
        emit q->connected();
    }
}

void QLowEnergyControllerPrivateAndroid::servicesDiscovered(
        QLowEnergyController::Error errorCode, const QString &foundServices)
{
    Q_Q(QLowEnergyController);

    if (errorCode == QLowEnergyController::NoError) {
        //Android delivers all services in one go
        const QStringList list = foundServices.split(QChar::Space, Qt::SkipEmptyParts);
        for (const QString &entry : list) {
            const QBluetoothUuid service(entry);
            if (service.isNull())
                return;

            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = service;
            priv->setController(this);

            QSharedPointer<QLowEnergyServicePrivate> pointer(priv);
            serviceList.insert(service, pointer);

            emit q->serviceDiscovered(QBluetoothUuid(entry));
        }

        setState(QLowEnergyController::DiscoveredState);
        emit q->discoveryFinished();
    } else {
        setError(errorCode);
        setState(QLowEnergyController::ConnectedState);
    }
}

void QLowEnergyControllerPrivateAndroid::serviceDetailsDiscoveryFinished(
        const QString &serviceUuid, int startHandle, int endHandle)
{
    const QBluetoothUuid service(serviceUuid);
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_ANDROID) << "Discovery done of unknown service:"
                                 << service.toString();
        return;
    }

    //update service data
    QSharedPointer<QLowEnergyServicePrivate> pointer =
            serviceList.value(service);
    pointer->startHandle = startHandle;
    pointer->endHandle = endHandle;

    if (hub && hub->javaObject().isValid()) {
        QJniObject uuid = QJniObject::fromString(serviceUuid);
        QJniObject javaIncludes = hub->javaObject().callMethod<jstring>(
                                        "includedServices", uuid.object<jstring>());
        if (javaIncludes.isValid()) {
            const QStringList list = javaIncludes.toString()
                                         .split(QChar::Space, Qt::SkipEmptyParts);
            for (const QString &entry : list) {
                const QBluetoothUuid service(entry);
                if (service.isNull())
                    return;

                pointer->includedServices.append(service);

                // update the type of the included service
                QSharedPointer<QLowEnergyServicePrivate> otherService =
                        serviceList.value(service);
                if (!otherService.isNull())
                    otherService->type |= QLowEnergyService::IncludedService;
            }
        }
    }

    qCDebug(QT_BT_ANDROID) << "Service" << serviceUuid << "discovered (start:"
              << startHandle << "end:" << endHandle << ")" << pointer.data();

    pointer->setState(QLowEnergyService::RemoteServiceDiscovered);
}

void QLowEnergyControllerPrivateAndroid::characteristicRead(
        const QBluetoothUuid &serviceUuid, int handle,
        const QBluetoothUuid &charUuid, int properties, const QByteArray &data)
{
    if (!serviceList.contains(serviceUuid))
        return;

    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceList.value(serviceUuid);
    QLowEnergyHandle charHandle = handle;

    QLowEnergyServicePrivate::CharData &charDetails =
            service->characteristicList[charHandle];

    //Android uses same property value as Qt which is the Bluetooth LE standard
    charDetails.properties = QLowEnergyCharacteristic::PropertyType(properties);
    charDetails.uuid = charUuid;
    charDetails.value = data;
    //value handle always one larger than characteristics value handle
    charDetails.valueHandle = charHandle + 1;

    if (service->state == QLowEnergyService::RemoteServiceDiscovered) {
        QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
        if (!characteristic.isValid()) {
            qCWarning(QT_BT_ANDROID) << "characteristicRead: Cannot find characteristic";
            return;
        }
        emit service->characteristicRead(characteristic, data);
    }
}

void QLowEnergyControllerPrivateAndroid::descriptorRead(
        const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
        int descHandle, const QBluetoothUuid &descUuid, const QByteArray &data)
{
    if (!serviceList.contains(serviceUuid))
        return;

    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceList.value(serviceUuid);

    bool entryUpdated = false;

    CharacteristicDataMap::iterator charIt = service->characteristicList.begin();
    for ( ; charIt != service->characteristicList.end(); ++charIt) {
        QLowEnergyServicePrivate::CharData &charDetails = charIt.value();

        if (charDetails.uuid != charUuid)
            continue;

        // new entry created if it doesn't exist
        QLowEnergyServicePrivate::DescData &descDetails =
                charDetails.descriptorList[descHandle];
        descDetails.uuid = descUuid;
        descDetails.value = data;
        entryUpdated = true;
        break;
    }

    if (!entryUpdated) {
        qCWarning(QT_BT_ANDROID) << "Cannot find/update descriptor"
                                 << descUuid << charUuid << serviceUuid;
    } else if (service->state == QLowEnergyService::RemoteServiceDiscovered){
        QLowEnergyDescriptor descriptor = descriptorForHandle(descHandle);
        if (!descriptor.isValid()) {
            qCWarning(QT_BT_ANDROID) << "descriptorRead: Cannot find descriptor";
            return;
        }
        emit service->descriptorRead(descriptor, data);
    }
}

void QLowEnergyControllerPrivateAndroid::characteristicWritten(
        int charHandle, const QByteArray &data, QLowEnergyService::ServiceError errorCode)
{
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(charHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_ANDROID) << "Characteristic write confirmation" << service->uuid
                           << charHandle << data.toHex() << errorCode;

    if (errorCode != QLowEnergyService::NoError) {
        service->setError(errorCode);
        return;
    }

    QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_ANDROID) << "characteristicWritten: Cannot find characteristic";
        return;
    }

    // only update cache when property is readable. Otherwise it remains
    // empty.
    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(charHandle, data, false);
    emit service->characteristicWritten(characteristic, data);
}

void QLowEnergyControllerPrivateAndroid::descriptorWritten(
        int descHandle, const QByteArray &data, QLowEnergyService::ServiceError errorCode)
{
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(descHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_ANDROID) << "Descriptor write confirmation" << service->uuid
                           << descHandle << data.toHex() << errorCode;

    if (errorCode != QLowEnergyService::NoError) {
        service->setError(errorCode);
        return;
    }

    QLowEnergyDescriptor descriptor = descriptorForHandle(descHandle);
    if (!descriptor.isValid()) {
        qCWarning(QT_BT_ANDROID) << "descriptorWritten: Cannot find descriptor";
        return;
    }

    updateValueOfDescriptor(descriptor.characteristicHandle(),
                            descHandle, data, false);
    emit service->descriptorWritten(descriptor, data);
}

void QLowEnergyControllerPrivateAndroid::serverDescriptorWritten(
        const QJniObject &jniDesc, const QByteArray &newValue)
{
    qCDebug(QT_BT_ANDROID) << "Server descriptor change notification" << newValue.toHex();

    // retrieve service, char and desc uuids
    const QJniObject jniChar = jniDesc.callMethod<QtJniTypes::BluetoothGattCharacteristic>(
                                            "getCharacteristic");
    if (!jniChar.isValid())
        return;

    const QJniObject jniService =
            jniChar.callMethod<QtJniTypes::BluetoothGattService>("getService");
    if (!jniService.isValid())
        return;

    QJniObject jniUuid = jniService.callMethod<QtJniTypes::UUID>("getUuid");
    const QBluetoothUuid serviceUuid(jniUuid.toString());
    if (serviceUuid.isNull())
        return;

    // TODO test if two service with same uuid exist
    if (!localServices.contains(serviceUuid))
        return;

    jniUuid = jniChar.callMethod<QtJniTypes::UUID>("getUuid");
    const QBluetoothUuid characteristicUuid(jniUuid.toString());
    if (characteristicUuid.isNull())
        return;

    jniUuid = jniDesc.callMethod<QtJniTypes::UUID>("getUuid");
    const QBluetoothUuid descriptorUuid(jniUuid.toString());
    if (descriptorUuid.isNull())
        return;

    // find matching QLEDescriptor
    auto servicePrivate = localServices.value(serviceUuid);
    // TODO test if service contains two characteristics with same uuid
    // or characteristic contains two descriptors with same uuid
    const auto handleList = servicePrivate->characteristicList.keys();
    for (const auto charHandle: handleList) {
        const auto &charData = servicePrivate->characteristicList.value(charHandle);
        if (charData.uuid != characteristicUuid)
            continue;

        const auto &descHandleList = charData.descriptorList.keys();
        for (const auto descHandle: descHandleList) {
            const auto &descData = charData.descriptorList.value(descHandle);
            if (descData.uuid != descriptorUuid)
                continue;

            qCDebug(QT_BT_ANDROID) << "serverDescriptorChanged: Matching descriptor"
                                   << descriptorUuid << "in char" << characteristicUuid
                                   << "of service" << serviceUuid;

            servicePrivate->characteristicList[charHandle].descriptorList[descHandle].value = newValue;

            emit servicePrivate->descriptorWritten(
                        QLowEnergyDescriptor(servicePrivate, charHandle, descHandle),
                        newValue);
            return;
        }
    }
}

void QLowEnergyControllerPrivateAndroid::characteristicChanged(
        int charHandle, const QByteArray &data)
{
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(charHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_ANDROID) << "Characteristic change notification" << service->uuid
                           << charHandle << data.toHex() << "length:" << data.size();

    QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_ANDROID) << "characteristicChanged: Cannot find characteristic";
        return;
    }

    // only update cache when property is readable. Otherwise it remains
    // empty.
    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(characteristic.attributeHandle(),
                                data, false);
    emit service->characteristicChanged(characteristic, data);
}

void QLowEnergyControllerPrivateAndroid::serverCharacteristicChanged(
        const QJniObject &characteristic, const QByteArray &newValue)
{
    qCDebug(QT_BT_ANDROID) << "Server characteristic change notification"
        << newValue.toHex() << "length:" << newValue.size();

    // match characteristic to servicePrivate
    QJniObject service = characteristic.callMethod<QtJniTypes::BluetoothGattService>(
                                        "getService");
    if (!service.isValid())
        return;

    QJniObject jniUuid = service.callMethod<QtJniTypes::UUID>("getUuid");
    QBluetoothUuid serviceUuid(jniUuid.toString());
    if (serviceUuid.isNull())
        return;

    // TODO test if two service with same uuid exist
    if (!localServices.contains(serviceUuid))
        return;

    auto servicePrivate = localServices.value(serviceUuid);

    jniUuid = characteristic.callMethod<QtJniTypes::UUID>("getUuid");
    QBluetoothUuid characteristicUuid(jniUuid.toString());
    if (characteristicUuid.isNull())
        return;

    QLowEnergyHandle foundHandle = 0;
    const auto handleList = servicePrivate->characteristicList.keys();
    // TODO test if service contains two characteristics with same uuid
    for (const auto handle: handleList) {
        QLowEnergyServicePrivate::CharData &charData = servicePrivate->characteristicList[handle];
        if (charData.uuid != characteristicUuid)
            continue;

        qCDebug(QT_BT_ANDROID) << "serverCharacteristicChanged: Matching characteristic"
                               << characteristicUuid << " on " << serviceUuid;
        charData.value = newValue;
        foundHandle = handle;
        break;
    }

    if (!foundHandle)
        return;

    emit servicePrivate->characteristicChanged(
                QLowEnergyCharacteristic(servicePrivate, foundHandle), newValue);
}

void QLowEnergyControllerPrivateAndroid::serviceError(
        int attributeHandle, QLowEnergyService::ServiceError errorCode)
{
    // ignore call if it isn't really an error
    if (errorCode == QLowEnergyService::NoError)
        return;

    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(attributeHandle);
    Q_ASSERT(!service.isNull());

    // ATM we don't really use attributeHandle but later on we might
    // want to associate the error code with a char or desc
    service->setError(errorCode);
}

void QLowEnergyControllerPrivateAndroid::advertisementError(int errorCode)
{
    Q_Q(QLowEnergyController);

    switch (errorCode)
    {
    case 1: // AdvertiseCallback.ADVERTISE_FAILED_DATA_TOO_LARGE
        errorString = QLowEnergyController::tr("Advertisement data is larger than 31 bytes");
        break;
    case 2: // AdvertiseCallback.ADVERTISE_FAILED_FEATURE_UNSUPPORTED
        errorString = QLowEnergyController::tr("Advertisement feature not supported on the platform");
        break;
    case 3: // AdvertiseCallback.ADVERTISE_FAILED_INTERNAL_ERROR
        errorString = QLowEnergyController::tr("Error occurred trying to start advertising");
        break;
    case 4: // AdvertiseCallback.ADVERTISE_FAILED_TOO_MANY_ADVERTISERS
        errorString = QLowEnergyController::tr("Failed due to too many advertisers");
        break;
    default:
        errorString = QLowEnergyController::tr("Unknown advertisement error");
        break;
    }

    error = QLowEnergyController::AdvertisingError;
    emit q->errorOccurred(error);

    // not relevant states in peripheral mode
    Q_ASSERT(state != QLowEnergyController::DiscoveredState);
    Q_ASSERT(state != QLowEnergyController::DiscoveringState);

    switch (state)
    {
    case QLowEnergyController::UnconnectedState:
    case QLowEnergyController::ConnectingState:
    case QLowEnergyController::ConnectedState:
    case QLowEnergyController::ClosingState:
        // noop as remote is already connected or about to disconnect.
        // when connection drops we reset to unconnected anyway
        break;

    case QLowEnergyController::AdvertisingState:
        setState(QLowEnergyController::UnconnectedState);
        break;
    default:
        break;
    }
}

static QJniObject javaParcelUuidfromQtUuid(const QBluetoothUuid &uuid)
{
    QString output = uuid.toString();
    // cut off leading and trailing brackets
    output = output.mid(1, output.size()-2);

    QJniObject javaString = QJniObject::fromString(output);
    QJniObject parcelUuid = QJniObject::callStaticMethod<QtJniTypes::ParcelUuid>(
                QtJniTypes::Traits<QtJniTypes::ParcelUuid>::className(), "fromString",
                javaString.object<jstring>());

    return parcelUuid;
}

static QJniObject createJavaAdvertiseData(const QLowEnergyAdvertisingData &data)
{
    QJniObject builder = QJniObject::construct<QtJniTypes::AdvertiseDataBuilder>();

    // device name cannot be set but there is choice to show it or not
    builder = builder.callMethod<QtJniTypes::AdvertiseDataBuilder>(
                                "setIncludeDeviceName", !data.localName().isEmpty());
    builder = builder.callMethod<QtJniTypes::AdvertiseDataBuilder>(
                                "setIncludeTxPowerLevel", data.includePowerLevel());
    const auto services = data.services();
    for (const auto &service : services) {
        builder = builder.callMethod<QtJniTypes::AdvertiseDataBuilder>("addServiceUuid",
                               javaParcelUuidfromQtUuid(service).object<QtJniTypes::ParcelUuid>());
    }

    if (!data.manufacturerData().isEmpty()) {
        QJniEnvironment env;
        const jsize nativeSize = q26::saturate_cast<jsize>(data.manufacturerData().size());
        jbyteArray nativeData = env->NewByteArray(nativeSize);
        env->SetByteArrayRegion(nativeData, 0, nativeSize,
                                reinterpret_cast<const jbyte*>(data.manufacturerData().constData()));
        builder = builder.callMethod<QtJniTypes::AdvertiseDataBuilder>(
                                    "addManufacturerData", jint(data.manufacturerId()), nativeData);
        env->DeleteLocalRef(nativeData);

        if (!builder.isValid()) {
            qCWarning(QT_BT_ANDROID) << "Cannot set manufacturer id/data";
        }
    }

    /*// TODO Qt vs Java API mismatch
          -> Qt assumes rawData() is a global field
          -> Android pairs rawData() per service uuid
    if (!data.rawData().isEmpty()) {
        QJniEnvironment env;
        const jsize nativeSize = q26::saturate_cast<jsize>(data.rawData().size());
        jbyteArray nativeData = env->NewByteArray(nativeSize);
        env->SetByteArrayRegion(nativeData, 0, nativeSize,
                                reinterpret_cast<const jbyte*>(data.rawData().constData()));
        builder = builder.callObjectMethod("addServiceData",
                                       "(Landroid/os/ParcelUuid;[B])Landroid/bluetooth/le/AdvertiseData$Builder;",
                                       data.rawData().object(), nativeData);
        env->DeleteLocalRef(nativeData);

        if (env.checkAndClearExceptions()) {
            qCWarning(QT_BT_ANDROID) << "Cannot set advertisement raw data";
        }
    }*/

    QJniObject javaAdvertiseData = builder.callMethod<QtJniTypes::AdvertiseData>("build");
    return javaAdvertiseData;
}

static QJniObject createJavaAdvertiseSettings(const QLowEnergyAdvertisingParameters &params)
{
    QJniObject builder = QJniObject::construct<QtJniTypes::AdvertiseSettingsBuilder>();

    bool connectable = false;
    switch (params.mode())
    {
    case QLowEnergyAdvertisingParameters::AdvInd:
        connectable = true;
        break;
    case QLowEnergyAdvertisingParameters::AdvScanInd:
    case QLowEnergyAdvertisingParameters::AdvNonConnInd:
        connectable = false;
        break;
    // intentionally no default case
    }
    builder = builder.callMethod<QtJniTypes::AdvertiseSettingsBuilder>(
                      "setConnectable", connectable);

    /* TODO No Android API for further QLowEnergyAdvertisingParameters options
     *      Android TxPowerLevel, AdvertiseMode and Timeout not mappable to Qt
     */

    QJniObject javaAdvertiseSettings = builder.callMethod<QtJniTypes::AdvertiseSettings>("build");
    return javaAdvertiseSettings;
}


void QLowEnergyControllerPrivateAndroid::startAdvertising(const QLowEnergyAdvertisingParameters &params,
        const QLowEnergyAdvertisingData &advertisingData,
        const QLowEnergyAdvertisingData &scanResponseData)
{
    setState(QLowEnergyController::AdvertisingState);

    if (!ensureAndroidPermission(QBluetoothPermission::Access | QBluetoothPermission::Advertise)) {
        qCWarning(QT_BT_ANDROID) << "startAdvertising() failed due to missing permissions";
        setError(QLowEnergyController::MissingPermissionsError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    if (!hub || !hub->javaObject().isValid()) {
        qCWarning(QT_BT_ANDROID) << "Cannot initiate QtBluetoothLEServer";
        setError(QLowEnergyController::AdvertisingError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    // Pass on advertisingData, scanResponse & AdvertiseSettings
    QJniObject jAdvertiseData = createJavaAdvertiseData(advertisingData);
    QJniObject jScanResponse = createJavaAdvertiseData(scanResponseData);
    QJniObject jAdvertiseSettings = createJavaAdvertiseSettings(params);

    const bool result = hub->javaObject().callMethod<jboolean>("startAdvertising",
                        jAdvertiseData.object<QtJniTypes::AdvertiseData>(),
                        jScanResponse.object<QtJniTypes::AdvertiseData>(),
                        jAdvertiseSettings.object<QtJniTypes::AdvertiseSettings>());
    if (!result) {
        setError(QLowEnergyController::AdvertisingError);
        setState(QLowEnergyController::UnconnectedState);
    }
}

void QLowEnergyControllerPrivateAndroid::stopAdvertising()
{
    setState(QLowEnergyController::UnconnectedState);
    hub->javaObject().callMethod<void>("stopAdvertising");
}

void QLowEnergyControllerPrivateAndroid::requestConnectionUpdate(const QLowEnergyConnectionParameters &params)
{
    // Android does not permit specification of specific latency or min/max
    // connection intervals (see BluetoothGatt.requestConnectionPriority()
    // In fact, each device manufacturer is permitted to change those values via a config
    // file too. Therefore we can only make an approximated guess (see implementation below)
    // In addition there is no feedback signal (known bug) from the hardware layer as per v24.

    // TODO recheck in later Android releases whether callback for
    // BluetoothGatt.requestConnectionPriority() was added

    if (role != QLowEnergyController::CentralRole) {
        qCWarning(QT_BT_ANDROID) << "On Android, connection requests only work for central role";
        return;
    }

    const bool result = hub->javaObject().callMethod<jboolean>("requestConnectionUpdatePriority",
                                                                params.minimumInterval());
    if (!result)
        qCWarning(QT_BT_ANDROID) << "Cannot set connection update priority";
}

/*
 * Returns the Java char permissions based on the given characteristic data.
 */
static int setupCharPermissions(const QLowEnergyCharacteristicData &charData)
{
    int permission = 0;
    if (charData.properties() & QLowEnergyCharacteristic::Read) {
        if (int(charData.readConstraints()) == 0 // nothing is equivalent to simple read
            || (charData.readConstraints()
                & QBluetooth::AttAccessConstraint::AttAuthorizationRequired)) {
            permission |= QJniObject::getStaticField<jint>(
                           QtJniTypes::Traits<QtJniTypes::BluetoothGattCharacteristic>::className(),
                           "PERMISSION_READ");
        }

        if (charData.readConstraints()
            & QBluetooth::AttAccessConstraint::AttAuthenticationRequired) {
            permission |= QJniObject::getStaticField<jint>(
                           QtJniTypes::Traits<QtJniTypes::BluetoothGattCharacteristic>::className(),
                           "PERMISSION_READ_ENCRYPTED");
        }

        if (charData.readConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired) {
            permission |= QJniObject::getStaticField<jint>(
                           QtJniTypes::Traits<QtJniTypes::BluetoothGattCharacteristic>::className(),
                           "PERMISSION_READ_ENCRYPTED_MITM");
        }
    }

    if (charData.properties() &
                (QLowEnergyCharacteristic::Write|QLowEnergyCharacteristic::WriteNoResponse) ) {
        if (int(charData.writeConstraints()) == 0 // no flag is equivalent ti simple write
            || (charData.writeConstraints()
                & QBluetooth::AttAccessConstraint::AttAuthorizationRequired)) {
            permission |= QJniObject::getStaticField<jint>(
                           QtJniTypes::Traits<QtJniTypes::BluetoothGattCharacteristic>::className(),
                           "PERMISSION_WRITE");
        }

        if (charData.writeConstraints()
            & QBluetooth::AttAccessConstraint::AttAuthenticationRequired) {
            permission |= QJniObject::getStaticField<jint>(
                           QtJniTypes::Traits<QtJniTypes::BluetoothGattCharacteristic>::className(),
                           "PERMISSION_WRITE_ENCRYPTED");
        }

        if (charData.writeConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired) {
            permission |= QJniObject::getStaticField<jint>(
                           QtJniTypes::Traits<QtJniTypes::BluetoothGattCharacteristic>::className(),
                           "PERMISSION_WRITE_ENCRYPTED_MITM");
        }
    }

    if (charData.properties() & QLowEnergyCharacteristic::WriteSigned) {
        if (charData.writeConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired) {
            permission |= QJniObject::getStaticField<jint>(
                           QtJniTypes::Traits<QtJniTypes::BluetoothGattCharacteristic>::className(),
                           "PERMISSION_WRITE_SIGNED_MITM");
        } else {
            permission |= QJniObject::getStaticField<jint>(
                           QtJniTypes::Traits<QtJniTypes::BluetoothGattCharacteristic>::className(),
                           "PERMISSION_WRITE_SIGNED");
        }
    }

    return permission;
}

/*
 * Returns the Java desc permissions based on the given descriptor data.
 */
static int setupDescPermissions(const QLowEnergyDescriptorData &descData)
{
    int permissions = 0;

    if (descData.isReadable()) {
        if (int(descData.readConstraints()) == 0 // empty is equivalent to simple read
            || (descData.readConstraints()
                & QBluetooth::AttAccessConstraint::AttAuthorizationRequired)) {
            permissions |= QJniObject::getStaticField<jint>(
                               QtJniTypes::Traits<QtJniTypes::BluetoothGattDescriptor>::className(),
                               "PERMISSION_READ");
        }

        if (descData.readConstraints()
            & QBluetooth::AttAccessConstraint::AttAuthenticationRequired) {
            permissions |= QJniObject::getStaticField<jint>(
                               QtJniTypes::Traits<QtJniTypes::BluetoothGattDescriptor>::className(),
                               "PERMISSION_READ_ENCRYPTED");
        }

        if (descData.readConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired) {
            permissions |= QJniObject::getStaticField<jint>(
                               QtJniTypes::Traits<QtJniTypes::BluetoothGattDescriptor>::className(),
                               "PERMISSION_READ_ENCRYPTED_MITM");
        }
    }

    if (descData.isWritable()) {
        if (int(descData.readConstraints()) == 0 // empty is equivalent to simple read
            || (descData.readConstraints()
                & QBluetooth::AttAccessConstraint::AttAuthorizationRequired)) {
            permissions |= QJniObject::getStaticField<jint>(
                               QtJniTypes::Traits<QtJniTypes::BluetoothGattDescriptor>::className(),
                               "PERMISSION_WRITE");
        }

        if (descData.readConstraints()
            & QBluetooth::AttAccessConstraint::AttAuthenticationRequired) {
            permissions |= QJniObject::getStaticField<jint>(
                               QtJniTypes::Traits<QtJniTypes::BluetoothGattDescriptor>::className(),
                               "PERMISSION_WRITE_ENCRYPTED");
        }

        if (descData.readConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired) {
            permissions |= QJniObject::getStaticField<jint>(
                               QtJniTypes::Traits<QtJniTypes::BluetoothGattDescriptor>::className(),
                               "PERMISSION_WRITE_ENCRYPTED_MITM");
        }
    }

    return permissions;
}

void QLowEnergyControllerPrivateAndroid::addToGenericAttributeList(const QLowEnergyServiceData &serviceData,
                                                            QLowEnergyHandle startHandle)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(startHandle);
    if (service.isNull())
        return;

    // create BluetoothGattService object
    jint sType = QJniObject::getStaticField<jint>(
                            QtJniTypes::Traits<QtJniTypes::BluetoothGattService>::className(),
                            "SERVICE_TYPE_PRIMARY");
    if (serviceData.type() == QLowEnergyServiceData::ServiceTypeSecondary)
        sType = QJniObject::getStaticField<jint>(
                            QtJniTypes::Traits<QtJniTypes::BluetoothGattService>::className(),
                            "SERVICE_TYPE_SECONDARY");

    service->androidService = QJniObject::construct<QtJniTypes::BluetoothGattService>(
                javaUuidfromQtUuid(service->uuid).object<QtJniTypes::UUID>(), sType);

    // add included services, which must have been added earlier already
    const QList<QLowEnergyService*> includedServices = serviceData.includedServices();
    for (const auto includedServiceEntry: includedServices) {
        //TODO test this end-to-end
        const jboolean result = service->androidService.callMethod<jboolean>("addService",
                        includedServiceEntry->d_ptr->androidService.object<QtJniTypes::BluetoothGattService>());
        if (!result)
            qWarning(QT_BT_ANDROID) << "Cannot add included service " << includedServiceEntry->serviceUuid()
                                    << "to current service" << service->uuid;
    }

    // add characteristics
    const QList<QLowEnergyCharacteristicData> serviceCharsData = serviceData.characteristics();
    for (const auto &charData: serviceCharsData) {

        // User may have set minimum and/or maximum characteristic value size. Enforce
        // them here. If the user has not defined these limits, the default values 0..INT_MAX
        // do not limit anything. If the sizes are violated here (ie. when adding the
        // characteristics), regard it as a programming error.
        if (charData.value().size() < charData.minimumValueLength() ||
            charData.value().size() > charData.maximumValueLength()) {
            qWarning() << "Warning: Ignoring characteristic" << charData.uuid()
                       << "with invalid length:"  << charData.value().size()
                       << "(minimum:" << charData.minimumValueLength()
                       << "maximum:" << charData.maximumValueLength() << ").";
            continue;
        }

        // Issue a warning if the attribute length exceeds the bluetooth standard limit.
        if (charData.value().size() > BTLE_MAX_ATTRIBUTE_VALUE_SIZE) {
            qCWarning(QT_BT_ANDROID) << "Warning: characteristic" << charData.uuid() << "size"
                                     << "exceeds the standard: " << BTLE_MAX_ATTRIBUTE_VALUE_SIZE
                                     << ", value size:" << charData.value().size();
        }

        QJniObject javaChar = QJniObject::construct<QtJniTypes::QtBtGattCharacteristic>(
                           javaUuidfromQtUuid(charData.uuid()).object<QtJniTypes::UUID>(),
                           int(charData.properties()),
                           setupCharPermissions(charData),
                           charData.minimumValueLength(),
                           charData.maximumValueLength());

        QJniEnvironment env;
        const jsize nativeCharSize = q26::saturate_cast<jsize>(charData.value().size());
        jbyteArray jb = env->NewByteArray(nativeCharSize);
        env->SetByteArrayRegion(jb, 0, nativeCharSize, (jbyte*)charData.value().data());
        jboolean success = javaChar.callMethod<jboolean>("setLocalValue", jb);
        if (!success)
            qCWarning(QT_BT_ANDROID) << "Cannot setup initial characteristic value for " << charData.uuid();
        env->DeleteLocalRef(jb);

        const QList<QLowEnergyDescriptorData> descriptorList = charData.descriptors();
        for (const auto &descData: descriptorList) {
            QJniObject javaDesc = QJniObject::construct<QtJniTypes::QtBtGattDescriptor>(
                                    javaUuidfromQtUuid(descData.uuid()).object<QtJniTypes::UUID>(),
                                    setupDescPermissions(descData));

            const jsize nativeDescSize = q26::saturate_cast<jsize>(descData.value().size());
            jb = env->NewByteArray(nativeDescSize);
            env->SetByteArrayRegion(jb, 0, nativeDescSize, (jbyte*)descData.value().data());
            success = javaDesc.callMethod<jboolean>("setLocalValue", jb);
            if (!success) {
                qCWarning(QT_BT_ANDROID) << "Cannot setup initial descriptor value for "
                                         << descData.uuid() << "(char" << charData.uuid()
                                         << "on service " << service->uuid << ")";
            }

            env->DeleteLocalRef(jb);


            success = javaChar.callMethod<jboolean>("addDescriptor",
                                          javaDesc.object<QtJniTypes::BluetoothGattDescriptor>());
            if (!success) {
                qCWarning(QT_BT_ANDROID) << "Cannot add descriptor" << descData.uuid()
                                         << "to service" << service->uuid << "(char:"
                                         << charData.uuid() << ")";
            }
        }

        success = service->androidService.callMethod<jboolean>(
                                    "addCharacteristic",
                                    javaChar.object<QtJniTypes::BluetoothGattCharacteristic>());
        if (!success) {
            qCWarning(QT_BT_ANDROID) << "Cannot add characteristic" << charData.uuid()
                                     << "to service" << service->uuid;
        }
    }

    hub->javaObject().callMethod<void>("addService",
                            service->androidService.object<QtJniTypes::BluetoothGattService>());
}

int QLowEnergyControllerPrivateAndroid::mtu() const
{
    if (!hub) {
        qCWarning(QT_BT_ANDROID) << "could not determine MTU, hub is does not exist";
        return -1;
    } else {
        int result = hub->javaObject().callMethod<int>("mtu");
        qCDebug(QT_BT_ANDROID) << "MTU found to be" << result;
        return result;
    }
}

void QLowEnergyControllerPrivateAndroid::readRssi()
{
    if (!hub || !hub->javaObject().callMethod<jboolean>("readRemoteRssi")) {
        qCWarning(QT_BT_ANDROID) << "request to read RSSI failed";
        setError(QLowEnergyController::RssiReadError);
        return;
    }
}

QT_END_NAMESPACE
