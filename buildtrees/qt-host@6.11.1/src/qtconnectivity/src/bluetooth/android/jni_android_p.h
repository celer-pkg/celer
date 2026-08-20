// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef JNI_ANDROID_P_H
#define JNI_ANDROID_P_H

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

#include "qbluetooth.h"
#include <QtCore/QJniObject>
#include <QtCore/private/qglobal_p.h>
#include <QtCore/qcoreapplication_platform.h>

QT_BEGIN_NAMESPACE

// CLASS declaration implies also TYPE declaration
Q_DECLARE_JNI_CLASS(QtBtUtility,
                    "org/qtproject/qt/android/bluetooth/QtBluetoothUtility")
Q_DECLARE_JNI_CLASS(QtBtBroadcastReceiver,
                    "org/qtproject/qt/android/bluetooth/QtBluetoothBroadcastReceiver")
Q_DECLARE_JNI_CLASS(QtBtGattCharacteristic,
                    "org/qtproject/qt/android/bluetooth/QtBluetoothGattCharacteristic")
Q_DECLARE_JNI_CLASS(QtBtGattDescriptor,
                    "org/qtproject/qt/android/bluetooth/QtBluetoothGattDescriptor")
Q_DECLARE_JNI_CLASS(QtBtInputStreamThread,
                    "org/qtproject/qt/android/bluetooth/QtBluetoothInputStreamThread")
Q_DECLARE_JNI_CLASS(QtBtSocketServer, "org/qtproject/qt/android/bluetooth/QtBluetoothSocketServer")
Q_DECLARE_JNI_CLASS(QtBtLEServer, "org/qtproject/qt/android/bluetooth/QtBluetoothLEServer")
Q_DECLARE_JNI_CLASS(QtBtLECentral, "org/qtproject/qt/android/bluetooth/QtBluetoothLE")
Q_DECLARE_JNI_CLASS(BluetoothAdapter, "android/bluetooth/BluetoothAdapter")
Q_DECLARE_JNI_CLASS(ParcelUuid, "android/os/ParcelUuid")
Q_DECLARE_JNI_CLASS(AdvertiseDataBuilder, "android/bluetooth/le/AdvertiseData$Builder")
Q_DECLARE_JNI_CLASS(AdvertiseSettingsBuilder, "android/bluetooth/le/AdvertiseSettings$Builder")
Q_DECLARE_JNI_CLASS(BluetoothGattService, "android/bluetooth/BluetoothGattService")
Q_DECLARE_JNI_CLASS(BluetoothGattDescriptor, "android/bluetooth/BluetoothGattDescriptor")
Q_DECLARE_JNI_CLASS(BluetoothGattCharacteristic, "android/bluetooth/BluetoothGattCharacteristic")
Q_DECLARE_JNI_CLASS(BluetoothDevice, "android/bluetooth/BluetoothDevice")
Q_DECLARE_JNI_CLASS(IntentFilter, "android/content/IntentFilter")
Q_DECLARE_JNI_CLASS(AndroidContext, "android/content/Context")


Q_DECLARE_JNI_CLASS(BluetoothManager, "android/bluetooth/BluetoothManager")
Q_DECLARE_JNI_CLASS(AdvertiseData, "android/bluetooth/le/AdvertiseData")
Q_DECLARE_JNI_CLASS(AdvertiseSettings, "android/bluetooth/le/AdvertiseSettings")
Q_DECLARE_JNI_CLASS(InputStream, "java/io/InputStream")
Q_DECLARE_JNI_CLASS(OutputStream, "java/io/OutputStream")
Q_DECLARE_JNI_CLASS(BluetoothSocket, "android/bluetooth/BluetoothSocket")
Q_DECLARE_JNI_CLASS(BroadcastReceiver, "android/content/BroadcastReceiver")
Q_DECLARE_JNI_CLASS(BluetoothClass, "android/bluetooth/BluetoothClass")
Q_DECLARE_JNI_CLASS(Bundle, "android/os/Bundle")
Q_DECLARE_JNI_CLASS(List, "java/util/List")

// QLowEnergyHandle is a quint16, ensure it is interpreted as jint
template<>
constexpr auto QtJniTypes::Traits<QLowEnergyHandle>::signature()
{
    return QtJniTypes::Traits<jint>::signature();
}

enum JavaNames {
    BluetoothAdapter = 0,
    BluetoothDevice,
    ActionAclConnected,
    ActionAclDisconnected,
    ActionBondStateChanged,
    ActionDiscoveryStarted,
    ActionDiscoveryFinished,
    ActionFound,
    ActionScanModeChanged,
    ActionUuid,
    ExtraBondState,
    ExtraDevice,
    ExtraPairingKey,
    ExtraPairingVariant,
    ExtraRssi,
    ExtraScanMode,
    ExtraUuid
};

QString valueFromStaticFieldCache(const char *key, const char *className, const char *fieldName);


template<typename Klass, JavaNames Field>
QString valueForStaticField()
{
    constexpr auto className = QtJniTypes::Traits<Klass>::className();
    constexpr auto fieldName = []() -> auto {
        if constexpr (Field == JavaNames::ActionAclConnected)
            return QtJniTypes::CTString("ACTION_ACL_CONNECTED");
        else if constexpr (Field == ActionAclDisconnected)
            return QtJniTypes::CTString("ACTION_ACL_DISCONNECTED");
        else if constexpr (Field == ActionBondStateChanged)
            return QtJniTypes::CTString("ACTION_BOND_STATE_CHANGED");
        else if constexpr (Field == ActionDiscoveryStarted)
            return QtJniTypes::CTString("ACTION_DISCOVERY_STARTED");
        else if constexpr (Field == ActionDiscoveryFinished)
            return QtJniTypes::CTString("ACTION_DISCOVERY_FINISHED");
        else if constexpr (Field == ActionFound)
            return QtJniTypes::CTString("ACTION_FOUND");
        else if constexpr (Field == ActionScanModeChanged)
            return QtJniTypes::CTString("ACTION_SCAN_MODE_CHANGED");
        else if constexpr (Field == ActionUuid)
            return QtJniTypes::CTString("ACTION_UUID");
        else if constexpr (Field == ExtraBondState)
            return QtJniTypes::CTString("EXTRA_BOND_STATE");
        else if constexpr (Field == ExtraDevice)
            return QtJniTypes::CTString("EXTRA_DEVICE");
        else if constexpr (Field == ExtraPairingKey)
            return QtJniTypes::CTString("EXTRA_PAIRING_KEY");
        else if constexpr (Field == ExtraPairingVariant)
            return QtJniTypes::CTString("EXTRA_PAIRING_VARIANT");
        else if constexpr (Field == ExtraRssi)
            return QtJniTypes::CTString("EXTRA_RSSI");
        else if constexpr (Field == ExtraScanMode)
            return QtJniTypes::CTString("EXTRA_SCAN_MODE");
        else if constexpr (Field == ExtraUuid)
            return QtJniTypes::CTString("EXTRA_UUID");
        else
            static_assert(QtPrivate::value_dependent_false<Field>());
    }();

    return valueFromStaticFieldCache(className + fieldName, className.data(), fieldName.data());
}

QT_END_NAMESPACE

#endif // JNI_ANDROID_P_H
