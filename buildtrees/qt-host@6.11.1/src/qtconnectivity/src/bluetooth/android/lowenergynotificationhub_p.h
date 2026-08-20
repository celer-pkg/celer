// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef LOWENERGYNOTIFICATIONHUB_H
#define LOWENERGYNOTIFICATIONHUB_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QObject>
#include <QtCore/QReadWriteLock>
#include <QtCore/QJniObject>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyService>
#include <QtCore/private/qglobal_p.h>
#include "android/jni_android_p.h"
#include <jni.h>

#include <QtBluetooth/QLowEnergyCharacteristic>

QT_BEGIN_NAMESPACE

class LowEnergyNotificationHub : public QObject
{
    Q_OBJECT
public:
    explicit LowEnergyNotificationHub(const QBluetoothAddress &remote, bool isPeripheral,
                                      QObject *parent = nullptr);
    ~LowEnergyNotificationHub();

    static void lowEnergy_connectionChange(JNIEnv*, jobject, jlong qtObject,
                                           jint errorCode, jint newState);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_connectionChange,
                                                 leConnectionStateChange)

    static void lowEnergy_mtuChanged(JNIEnv*, jobject, jlong qtObject, jint mtu);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_mtuChanged, leMtuChanged)

    static void lowEnergy_servicesDiscovered(JNIEnv*, jobject, jlong qtObject,
                                             jint errorCode, jstring uuidList);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_servicesDiscovered,
                                                 leServicesDiscovered)

    static void lowEnergy_serviceDetailsDiscovered(JNIEnv *, jobject,
                                                   jlong qtObject, jstring uuid,
                                                   jint startHandle, jint endHandle);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_serviceDetailsDiscovered,
                                                 leServiceDetailDiscoveryFinished)

    static void lowEnergy_characteristicRead(JNIEnv*env, jobject, jlong qtObject,
                                             jstring serviceUuid,
                                             jint handle, jstring charUuid,
                                             jint properties, jbyteArray data);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_characteristicRead,
                                                 leCharacteristicRead)

    static void lowEnergy_descriptorRead(JNIEnv *env, jobject, jlong qtObject,
                                         jstring sUuid, jstring cUuid,
                                         jint handle, jstring dUuid, jbyteArray data);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_descriptorRead, leDescriptorRead)

    static void lowEnergy_characteristicWritten(JNIEnv *, jobject, jlong qtObject,
                                                jint charHandle, jbyteArray data,
                                                jint errorCode);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_characteristicWritten,
                                                 leCharacteristicWritten)

    static void lowEnergy_descriptorWritten(JNIEnv *, jobject, jlong qtObject,
                                            jint descHandle, jbyteArray data,
                                            jint errorCode);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_descriptorWritten,
                                                 leDescriptorWritten)

    static void lowEnergy_serverDescriptorWritten(JNIEnv *, jobject, jlong qtObject,
                                                  QtJniTypes::BluetoothGattDescriptor descriptor,
                                                  jbyteArray newValue);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_serverDescriptorWritten,
                                                 leServerDescriptorWritten)

    static void lowEnergy_characteristicChanged(JNIEnv *, jobject, jlong qtObject,
                                                jint charHandle, jbyteArray data);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_characteristicChanged,
                                                 leCharacteristicChanged)

    static void lowEnergy_serverCharacteristicChanged(JNIEnv *, jobject, jlong qtObject,
                                            QtJniTypes::BluetoothGattCharacteristic characteristic,
                                            jbyteArray newValue);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_serverCharacteristicChanged,
                                                 leServerCharacteristicChanged)

    static void lowEnergy_serviceError(JNIEnv *, jobject, jlong qtObject,
                                       jint attributeHandle, int errorCode);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_serviceError, leServiceError)

    static void lowEnergy_advertisementError(JNIEnv *, jobject, jlong qtObject, jint status);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_advertisementError,
                                                 leServerAdvertisementError)

    static void lowEnergy_remoteRssiRead(JNIEnv *, jobject, jlong qtObject, int rssi,
                                         bool success);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(lowEnergy_remoteRssiRead, leRemoteRssiRead);

    QJniObject javaObject()
    {
        return jBluetoothLe;
    }

signals:
    void connectionUpdated(QLowEnergyController::ControllerState newState,
            QLowEnergyController::Error errorCode);
    void mtuChanged(int mtu);
    void remoteRssiRead(int rssi, bool success);
    void servicesDiscovered(QLowEnergyController::Error errorCode, const QString &uuids);
    void serviceDetailsDiscoveryFinished(const QString& serviceUuid,
            int startHandle, int endHandle);
    void characteristicRead(const QBluetoothUuid &serviceUuid,
            int handle, const QBluetoothUuid &charUuid,
            int properties, const QByteArray &data);
    void descriptorRead(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
            int handle, const QBluetoothUuid &descUuid, const QByteArray &data);
    void characteristicWritten(int charHandle, const QByteArray &data,
                               QLowEnergyService::ServiceError errorCode);
    void descriptorWritten(int descHandle, const QByteArray &data,
                           QLowEnergyService::ServiceError errorCode);
    void serverDescriptorWritten(const QJniObject &descriptor, const QByteArray &newValue);
    void characteristicChanged(int charHandle, const QByteArray &data);
    void serverCharacteristicChanged(const QJniObject &characteristic, const QByteArray &newValue);
    void serviceError(int attributeHandle, QLowEnergyService::ServiceError errorCode);
    void advertisementError(int status);

private:
    static QReadWriteLock lock;

    QJniObject jBluetoothLe;
    long javaToCtoken;

};

QT_END_NAMESPACE

#endif // LOWENERGYNOTIFICATIONHUB_H

