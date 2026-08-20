// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "android/servicediscoverybroadcastreceiver_p.h"
#include <QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QJniEnvironment>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include "android/jni_android_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

ServiceDiscoveryBroadcastReceiver::ServiceDiscoveryBroadcastReceiver(QObject* parent): AndroidBroadcastReceiver(parent)
{
    addAction(QJniObject::fromString(
        valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ActionUuid>()));
}

void ServiceDiscoveryBroadcastReceiver::onReceive(JNIEnv *env, jobject context, jobject intent)
{
    Q_UNUSED(context);
    Q_UNUSED(env);

    QJniObject intentObject(intent);
    const QString action = intentObject.callMethod<QString>("getAction");

    qCDebug(QT_BT_ANDROID) << "ServiceDiscoveryBroadcastReceiver::onReceive() - event:" << action;

    if (action == valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ActionUuid>()) {
        QString keyExtra = valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ExtraUuid>();
        const QJniArray parcelableUuids = intentObject.callMethod<QtJniTypes::Parcelable[]>(
                                                "getParcelableArrayExtra", keyExtra);
        if (!parcelableUuids.isValid()) {
            emit uuidFetchFinished(QBluetoothAddress(), QList<QBluetoothUuid>());
            return;
        }
        const QList<QBluetoothUuid> result = ServiceDiscoveryBroadcastReceiver::convertParcelableArray(parcelableUuids);

        keyExtra = valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ExtraDevice>();
        const QtJniTypes::Parcelable bluetoothDevice =
            intentObject.callMethod<QtJniTypes::Parcelable>("getParcelableExtra", keyExtra);
        QBluetoothAddress address;
        if (bluetoothDevice.isValid()) {
            address = QBluetoothAddress(bluetoothDevice.callMethod<QString>("getAddress"));
            emit uuidFetchFinished(address, result);
        } else {
            emit uuidFetchFinished(QBluetoothAddress(), QList<QBluetoothUuid>());
        }
    }
}

QList<QBluetoothUuid> ServiceDiscoveryBroadcastReceiver::convertParcelableArray(const QJniArray<QJniObject> &parcelUuidArray)
{
    QList<QBluetoothUuid> result;
    if (!parcelUuidArray.isValid())
        return result;

    for (const auto &parcel : parcelUuidArray) {
        QBluetoothUuid uuid(parcel.callMethod<QString>("toString"));
        //qCDebug(QT_BT_ANDROID) << uuid.toString();
        result.append(uuid);
    }

    return result;
}

QT_END_NAMESPACE

