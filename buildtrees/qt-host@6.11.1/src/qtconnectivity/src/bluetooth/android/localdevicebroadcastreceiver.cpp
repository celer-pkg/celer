// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QLoggingCategory>
#include <QtCore/qrandom.h>
#include "localdevicebroadcastreceiver_p.h"
#include "android/jni_android_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

const char *scanModes[] = {"SCAN_MODE_NONE", "SCAN_MODE_CONNECTABLE", "SCAN_MODE_CONNECTABLE_DISCOVERABLE"};
const char *bondModes[] = {"BOND_NONE", "BOND_BONDING", "BOND_BONDED"};

LocalDeviceBroadcastReceiver::LocalDeviceBroadcastReceiver(QObject *parent) :
    AndroidBroadcastReceiver(parent), previousScanMode(0)
{
    addAction(QJniObject::fromString(
        valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ActionBondStateChanged>()));
    addAction(QJniObject::fromString(
        valueForStaticField<QtJniTypes::BluetoothAdapter, JavaNames::ActionScanModeChanged>()));
    addAction(QJniObject::fromString(
        valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ActionAclConnected>()));
    addAction(QJniObject::fromString(
        valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ActionAclDisconnected>()));

    //cache integer values for host & bonding mode
    //don't use the java fields directly but refer to them by name
    for (uint i = 0; i < (sizeof(hostModePreset)/sizeof(hostModePreset[0])); i++) {
        hostModePreset[i] = QJniObject::getStaticField<jint>(
                                    QtJniTypes::Traits<QtJniTypes::BluetoothAdapter>::className(),
                                    scanModes[i]);
    }

    for (uint i = 0; i < (sizeof(bondingModePreset)/sizeof(bondingModePreset[0])); i++) {
        bondingModePreset[i] = QJniObject::getStaticField<jint>(
                                    QtJniTypes::Traits<QtJniTypes::BluetoothDevice>::className(),
                                    bondModes[i]);
    }
}

void LocalDeviceBroadcastReceiver::onReceive(JNIEnv *env, jobject context, jobject intent)
{
    Q_UNUSED(context);
    Q_UNUSED(env);

    QJniObject intentObject(intent);
    const QString action = intentObject.callMethod<jstring>("getAction").toString();
    qCDebug(QT_BT_ANDROID) << QStringLiteral("LocalDeviceBroadcastReceiver::onReceive() - event: %1").arg(action);

    if (action == valueForStaticField<QtJniTypes::BluetoothAdapter,
                                      JavaNames::ActionScanModeChanged>()) {

        const QJniObject extrasBundle =
                intentObject.callMethod<QtJniTypes::Bundle>("getExtras");
        const QJniObject keyExtra =
            QJniObject::fromString(valueForStaticField<QtJniTypes::BluetoothAdapter,
                                                       JavaNames::ExtraScanMode>());

        int extra = extrasBundle.callMethod<jint>("getInt", keyExtra.object<jstring>());

        if (previousScanMode != extra) {
            previousScanMode = extra;

            if (extra == hostModePreset[0])
                emit hostModeStateChanged(QBluetoothLocalDevice::HostPoweredOff);
            else if (extra == hostModePreset[1])
                emit hostModeStateChanged(QBluetoothLocalDevice::HostConnectable);
            else if (extra == hostModePreset[2])
                emit hostModeStateChanged(QBluetoothLocalDevice::HostDiscoverable);
            else
                qCWarning(QT_BT_ANDROID) << "Unknown Host State";
        }
    } else if (action == valueForStaticField<QtJniTypes::BluetoothDevice,
                                                        JavaNames::ActionBondStateChanged>()) {
        //get BluetoothDevice
        QJniObject keyExtra = QJniObject::fromString(
                                        valueForStaticField<QtJniTypes::BluetoothDevice,
                                        JavaNames::ExtraDevice>());
        const QJniObject bluetoothDevice =
                intentObject.callMethod<QtJniTypes::Parcelable>("getParcelableExtra",
                                                                keyExtra.object<jstring>());

        //get new bond state
        keyExtra = QJniObject::fromString(valueForStaticField<QtJniTypes::BluetoothDevice,
                                                              JavaNames::ExtraBondState>());
        const QJniObject extrasBundle =
                intentObject.callMethod<QtJniTypes::Bundle>("getExtras");
        int bondState = extrasBundle.callMethod<jint>("getInt", keyExtra.object<jstring>());

        QBluetoothAddress address(bluetoothDevice.callMethod<jstring>("getAddress").toString());
        if (address.isNull())
                return;

        if (bondState == bondingModePreset[0])
            emit pairingStateChanged(address, QBluetoothLocalDevice::Unpaired);
        else if (bondState == bondingModePreset[1])
            ; //we ignore this as Qt doesn't have equivalent API value
        else if (bondState == bondingModePreset[2])
            emit pairingStateChanged(address, QBluetoothLocalDevice::Paired);
        else
            qCWarning(QT_BT_ANDROID) << "Unknown BOND_STATE_CHANGED value:" << bondState;

    } else if (action == valueForStaticField<QtJniTypes::BluetoothDevice,
                                             JavaNames::ActionAclConnected>() ||
               action == valueForStaticField<QtJniTypes::BluetoothDevice,
                                             JavaNames::ActionAclDisconnected>()) {

        const QString connectEvent = valueForStaticField<QtJniTypes::BluetoothDevice,
                                                         JavaNames::ActionAclConnected>();
        const bool isConnectEvent =
                action == connectEvent ? true : false;

        //get BluetoothDevice
        const QJniObject keyExtra =
            QJniObject::fromString(valueForStaticField<QtJniTypes::BluetoothDevice,
                                                       JavaNames::ExtraDevice>());
        QJniObject bluetoothDevice =
                intentObject.callMethod<QtJniTypes::Parcelable>("getParcelableExtra",
                                                                keyExtra.object<jstring>());

        QBluetoothAddress address(bluetoothDevice.callMethod<jstring>("getAddress").toString());
        if (address.isNull())
            return;

        emit connectDeviceChanges(address, isConnectEvent);
    }
}

QT_END_NAMESPACE
