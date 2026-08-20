// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "android/androidbroadcastreceiver_p.h"
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothLocalDevice>

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

#ifndef LOCALDEVICEBROADCASTRECEIVER_H
#define LOCALDEVICEBROADCASTRECEIVER_H

QT_BEGIN_NAMESPACE

class LocalDeviceBroadcastReceiver : public AndroidBroadcastReceiver
{
    Q_OBJECT
public:
    explicit LocalDeviceBroadcastReceiver(QObject *parent = nullptr);
    virtual ~LocalDeviceBroadcastReceiver() {}
    virtual void onReceive(JNIEnv *env, jobject context, jobject intent);
    virtual void onReceiveLeScan(JNIEnv *, jobject, jint, jbyteArray) {}

signals:
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode state);
    void pairingStateChanged(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);
    void connectDeviceChanges(const QBluetoothAddress &address, bool isConnectEvent);

private:
    int previousScanMode;
    QJniObject pairingDevice;

    int bondingModePreset[3];
    int hostModePreset[3];
};

QT_END_NAMESPACE

#endif // LOCALDEVICEBROADCASTRECEIVER_H
