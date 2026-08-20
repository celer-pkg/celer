// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DEVICEDISCOVERYBROADCASTRECEIVER_H
#define DEVICEDISCOVERYBROADCASTRECEIVER_H

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

#include "android/androidbroadcastreceiver_p.h"
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>

QT_BEGIN_NAMESPACE

class QBluetoothDeviceInfo;

class DeviceDiscoveryBroadcastReceiver : public AndroidBroadcastReceiver
{
    Q_OBJECT
public:
    DeviceDiscoveryBroadcastReceiver(QObject* parent = nullptr);
    virtual void onReceive(JNIEnv *env, jobject context, jobject intent);
    virtual void onReceiveLeScan(JNIEnv *env, jobject jBluetoothDevice, jint rssi,
                                 jbyteArray scanRecord);

signals:
    void deviceDiscovered(const QBluetoothDeviceInfo &info, bool isLeScanResult);
    void discoveryStarted();
    void finished();

private:
    QBluetoothDeviceInfo retrieveDeviceInfo(const QJniObject& bluetoothDevice,
                                            int rssi, jbyteArray scanRecord = nullptr);
};

QT_END_NAMESPACE
#endif // DEVICEDISCOVERYBROADCASTRECEIVER_H
