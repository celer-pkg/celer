// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "lowenergynotificationhub_p.h"
#include "android/jni_android_p.h"

#include <QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>
#include <QtCore/QRandomGenerator>
#include <QtCore/QJniEnvironment>

QT_BEGIN_NAMESPACE

typedef QHash<long, LowEnergyNotificationHub*> HubMapType;
Q_GLOBAL_STATIC(HubMapType, hubMap)

QReadWriteLock LowEnergyNotificationHub::lock;

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

LowEnergyNotificationHub::LowEnergyNotificationHub(const QBluetoothAddress &remote,
                                                   bool isPeripheral, QObject *parent)
    :   QObject(parent), javaToCtoken(0)
{
    QJniEnvironment env;

    if (isPeripheral) {
        qCDebug(QT_BT_ANDROID) << "Creating Android Peripheral/Server support for BTLE";
        jBluetoothLe = QJniObject::construct<QtJniTypes::QtBtLEServer>(
                    QNativeInterface::QAndroidApplication::context());
    } else {
        qCDebug(QT_BT_ANDROID) << "Creating Android Central/Client support for BTLE";
        const QJniObject address =
            QJniObject::fromString(remote.toString());
        jBluetoothLe = QJniObject::construct<QtJniTypes::QtBtLECentral>(
                    address.object<jstring>(), QNativeInterface::QAndroidApplication::context());
    }

    if (!jBluetoothLe.isValid()) return;

    // register C++ class pointer in Java
    lock.lockForWrite();

    while (true) {
        javaToCtoken = QRandomGenerator::global()->generate();
        if (!hubMap()->contains(javaToCtoken))
            break;
    }

    hubMap()->insert(javaToCtoken, this);
    lock.unlock();

    jBluetoothLe.setField("qtObject", jlong(javaToCtoken));
}

LowEnergyNotificationHub::~LowEnergyNotificationHub()
{
    lock.lockForWrite();
    hubMap()->remove(javaToCtoken);
    lock.unlock();
}

// runs in Java thread
void LowEnergyNotificationHub::lowEnergy_connectionChange(JNIEnv *, jobject, jlong qtObject, jint errorCode, jint newState)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QMetaObject::invokeMethod(hub, "connectionUpdated", Qt::QueuedConnection,
                              Q_ARG(QLowEnergyController::ControllerState,
                                    (QLowEnergyController::ControllerState)newState),
                              Q_ARG(QLowEnergyController::Error,
                                    (QLowEnergyController::Error)errorCode));
}

void LowEnergyNotificationHub::lowEnergy_mtuChanged(
        JNIEnv *, jobject, jlong qtObject, jint mtu)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QMetaObject::invokeMethod(hub, "mtuChanged", Qt::QueuedConnection, Q_ARG(int, mtu));
}

void LowEnergyNotificationHub::lowEnergy_remoteRssiRead(JNIEnv *, jobject, jlong qtObject,
                                                        int rssi, bool success)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QMetaObject::invokeMethod(hub, "remoteRssiRead", Qt::QueuedConnection,
                              Q_ARG(int, rssi), Q_ARG(bool, success));
}


void LowEnergyNotificationHub::lowEnergy_servicesDiscovered(
        JNIEnv *, jobject, jlong qtObject, jint errorCode, jstring uuidList)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    const QString uuids = QJniObject(uuidList).toString();
    QMetaObject::invokeMethod(hub, "servicesDiscovered", Qt::QueuedConnection,
                              Q_ARG(QLowEnergyController::Error,
                                    (QLowEnergyController::Error)errorCode),
                              Q_ARG(QString, uuids));
}

void LowEnergyNotificationHub::lowEnergy_serviceDetailsDiscovered(
        JNIEnv *, jobject, jlong qtObject, jstring uuid, jint startHandle,
        jint endHandle)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    const QString serviceUuid = QJniObject(uuid).toString();
    QMetaObject::invokeMethod(hub, "serviceDetailsDiscoveryFinished",
                              Qt::QueuedConnection,
                              Q_ARG(QString, serviceUuid),
                              Q_ARG(int, startHandle),
                              Q_ARG(int, endHandle));
}

void LowEnergyNotificationHub::lowEnergy_characteristicRead(
        JNIEnv *env, jobject, jlong qtObject, jstring sUuid, jint handle,
        jstring cUuid, jint properties, jbyteArray data)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    const QBluetoothUuid serviceUuid(QJniObject(sUuid).toString());
    if (serviceUuid.isNull())
        return;

    const QBluetoothUuid charUuid(QJniObject(cUuid).toString());
    if (charUuid.isNull())
        return;

    QByteArray payload;
    if (data) { //empty Java byte array is 0x0
        jsize length = env->GetArrayLength(data);
        payload.resize(length);
        env->GetByteArrayRegion(data, 0, length,
                                reinterpret_cast<signed char*>(payload.data()));
    }

    QMetaObject::invokeMethod(hub, "characteristicRead", Qt::QueuedConnection,
                              Q_ARG(QBluetoothUuid, serviceUuid),
                              Q_ARG(int, handle),
                              Q_ARG(QBluetoothUuid, charUuid),
                              Q_ARG(int, properties),
                              Q_ARG(QByteArray, payload));

}

void LowEnergyNotificationHub::lowEnergy_descriptorRead(
        JNIEnv *env, jobject, jlong qtObject, jstring sUuid, jstring cUuid,
        jint handle, jstring dUuid, jbyteArray data)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    const QBluetoothUuid serviceUuid(QJniObject(sUuid).toString());
    if (serviceUuid.isNull())
        return;

    const QBluetoothUuid charUuid(QJniObject(cUuid).toString());
    const QBluetoothUuid descUuid(QJniObject(dUuid).toString());
    if (charUuid.isNull() || descUuid.isNull())
        return;

    QByteArray payload;
    if (data) { //empty Java byte array is 0x0
        jsize length = env->GetArrayLength(data);
        payload.resize(length);
        env->GetByteArrayRegion(data, 0, length,
                                reinterpret_cast<signed char*>(payload.data()));
    }

    QMetaObject::invokeMethod(hub, "descriptorRead", Qt::QueuedConnection,
                              Q_ARG(QBluetoothUuid, serviceUuid),
                              Q_ARG(QBluetoothUuid, charUuid),
                              Q_ARG(int, handle),
                              Q_ARG(QBluetoothUuid, descUuid),
                              Q_ARG(QByteArray, payload));
}

void LowEnergyNotificationHub::lowEnergy_characteristicWritten(
        JNIEnv *env, jobject, jlong qtObject, jint charHandle,
        jbyteArray data, jint errorCode)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QByteArray payload;
    if (data) { //empty Java byte array is 0x0
        jsize length = env->GetArrayLength(data);
        payload.resize(length);
        env->GetByteArrayRegion(data, 0, length,
                                reinterpret_cast<signed char*>(payload.data()));
    }

    QMetaObject::invokeMethod(hub, "characteristicWritten", Qt::QueuedConnection,
                              Q_ARG(int, charHandle),
                              Q_ARG(QByteArray, payload),
                              Q_ARG(QLowEnergyService::ServiceError,
                                    (QLowEnergyService::ServiceError)errorCode));
}

void LowEnergyNotificationHub::lowEnergy_descriptorWritten(
        JNIEnv *env, jobject, jlong qtObject, jint descHandle,
        jbyteArray data, jint errorCode)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QByteArray payload;
    if (data) { //empty Java byte array is 0x0
        jsize length = env->GetArrayLength(data);
        payload.resize(length);
        env->GetByteArrayRegion(data, 0, length,
                                reinterpret_cast<signed char*>(payload.data()));
    }

    QMetaObject::invokeMethod(hub, "descriptorWritten", Qt::QueuedConnection,
                              Q_ARG(int, descHandle),
                              Q_ARG(QByteArray, payload),
                              Q_ARG(QLowEnergyService::ServiceError,
                                    (QLowEnergyService::ServiceError)errorCode));
}

void LowEnergyNotificationHub::lowEnergy_serverDescriptorWritten(
        JNIEnv *env, jobject, jlong qtObject, QtJniTypes::BluetoothGattDescriptor descriptor,
        jbyteArray newValue)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QByteArray payload;
    if (newValue) { //empty Java byte array is 0x0
        jsize length = env->GetArrayLength(newValue);
        payload.resize(length);
        env->GetByteArrayRegion(newValue, 0, length,
                                reinterpret_cast<signed char*>(payload.data()));
    }

    QMetaObject::invokeMethod(hub, "serverDescriptorWritten", Qt::QueuedConnection,
                              Q_ARG(QJniObject, descriptor),
                              Q_ARG(QByteArray, payload));
}

void LowEnergyNotificationHub::lowEnergy_characteristicChanged(
        JNIEnv *env, jobject, jlong qtObject, jint charHandle, jbyteArray data)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QByteArray payload;
    if (data) { //empty Java byte array is 0x0
        jsize length = env->GetArrayLength(data);
        payload.resize(length);
        env->GetByteArrayRegion(data, 0, length,
                                reinterpret_cast<signed char*>(payload.data()));
    }

    QMetaObject::invokeMethod(hub, "characteristicChanged", Qt::QueuedConnection,
                              Q_ARG(int, charHandle), Q_ARG(QByteArray, payload));
}

void LowEnergyNotificationHub::lowEnergy_serverCharacteristicChanged(
        JNIEnv *env, jobject, jlong qtObject,
        QtJniTypes::BluetoothGattCharacteristic characteristic, jbyteArray newValue)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QByteArray payload;
    if (newValue) { //empty Java byte array is 0x0
        jsize length = env->GetArrayLength(newValue);
        payload.resize(length);
        env->GetByteArrayRegion(newValue, 0, length,
                                reinterpret_cast<signed char*>(payload.data()));
    }

    QMetaObject::invokeMethod(hub, "serverCharacteristicChanged", Qt::QueuedConnection,
                              Q_ARG(QJniObject, characteristic),
                              Q_ARG(QByteArray, payload));
}

void LowEnergyNotificationHub::lowEnergy_serviceError(
        JNIEnv *, jobject, jlong qtObject, jint attributeHandle, int errorCode)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QMetaObject::invokeMethod(hub, "serviceError", Qt::QueuedConnection,
                              Q_ARG(int, attributeHandle),
                              Q_ARG(QLowEnergyService::ServiceError,
                                    (QLowEnergyService::ServiceError)errorCode));
}

void LowEnergyNotificationHub::lowEnergy_advertisementError(
        JNIEnv *, jobject, jlong qtObject, jint status)
{
    lock.lockForRead();
    LowEnergyNotificationHub *hub = hubMap()->value(qtObject);
    lock.unlock();
    if (!hub)
        return;

    QMetaObject::invokeMethod(hub, "advertisementError", Qt::QueuedConnection,
                              Q_ARG(int, status));
}

QT_END_NAMESPACE
