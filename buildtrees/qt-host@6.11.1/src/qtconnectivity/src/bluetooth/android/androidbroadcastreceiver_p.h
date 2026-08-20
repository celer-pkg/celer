// Copyright (C) 2017 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef JNIBROADCASTRECEIVER_H
#define JNIBROADCASTRECEIVER_H

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

#include <jni.h>
#include <QtCore/QObject>
#include <QtCore/private/qglobal_p.h>
#include <android/log.h>
#include <android/jni_android_p.h>
#include <QtCore/QJniObject>

QT_BEGIN_NAMESPACE

void QtBroadcastReceiver_jniOnReceive(JNIEnv *, jobject, jlong,
                                      QtJniTypes::Context, QtJniTypes::Intent);

class AndroidBroadcastReceiver: public QObject
{
    Q_OBJECT
public:
    AndroidBroadcastReceiver(QObject* parent = nullptr);
    virtual ~AndroidBroadcastReceiver();

    void addAction(const QJniObject &filter);
    bool isValid() const;
    void unregisterReceiver();

protected:
    friend void QtBroadcastReceiver_jniOnReceive(JNIEnv *, jobject, jlong, QtJniTypes::Context,
                                                 QtJniTypes::Intent);
    virtual void onReceive(JNIEnv *env, jobject context, jobject intent) = 0;
    friend void QtBluetoothLE_leScanResult(JNIEnv *, jobject, jlong, QtJniTypes::BluetoothDevice,
                                           jint, jbyteArray);
    virtual void onReceiveLeScan(JNIEnv *env, jobject jBluetoothDevice, jint rssi, jbyteArray scanRecord) = 0;


    QJniObject contextObject;
    QJniObject intentFilterObject;
    QJniObject broadcastReceiverObject;
    bool valid;
};

QT_END_NAMESPACE
#endif // JNIBROADCASTRECEIVER_H
