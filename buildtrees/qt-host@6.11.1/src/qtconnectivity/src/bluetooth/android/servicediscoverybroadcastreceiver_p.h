// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SERVICEDISCOVERYBROADCASTRECEIVER_H
#define SERVICEDISCOVERYBROADCASTRECEIVER_H

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
#include <QtCore/QList>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothUuid>

QT_BEGIN_NAMESPACE

class QBluetoothDeviceInfo;

class ServiceDiscoveryBroadcastReceiver : public AndroidBroadcastReceiver
{
    Q_OBJECT
public:
    ServiceDiscoveryBroadcastReceiver(QObject* parent = nullptr);
    virtual void onReceive(JNIEnv *env, jobject context, jobject intent);
    virtual void onReceiveLeScan(JNIEnv *, jobject, jint, jbyteArray) {}

    static QList<QBluetoothUuid> convertParcelableArray(const QJniArray<QJniObject> &obj);

signals:
    void uuidFetchFinished(const QBluetoothAddress &addr, const QList<QBluetoothUuid> &serviceUuid);
};

QT_END_NAMESPACE
#endif // SERVICEDISCOVERYBROADCASTRECEIVER_H
