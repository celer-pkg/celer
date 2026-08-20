// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <android/log.h>
#include <android/jni_android_p.h>
#include "android/androidbroadcastreceiver_p.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/qnativeinterface.h>
#include <QtGui/QGuiApplication>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)


AndroidBroadcastReceiver::AndroidBroadcastReceiver(QObject* parent)
    : QObject(parent), valid(false)
{
    // get Qt Context
    contextObject = QJniObject(QNativeInterface::QAndroidApplication::context());

    broadcastReceiverObject = QJniObject::construct<QtJniTypes::QtBtBroadcastReceiver>();
    if (!broadcastReceiverObject.isValid())
        return;
    broadcastReceiverObject.setField("qtObject", reinterpret_cast<jlong>(this));

    intentFilterObject = QJniObject::construct<QtJniTypes::IntentFilter>();
    if (!intentFilterObject.isValid())
        return;

    valid = true;
}

AndroidBroadcastReceiver::~AndroidBroadcastReceiver()
{
}

bool AndroidBroadcastReceiver::isValid() const
{
    return valid;
}

void AndroidBroadcastReceiver::unregisterReceiver()
{
    if (!valid)
        return;

    broadcastReceiverObject.callMethod<void>("unregisterReceiver");
}

void AndroidBroadcastReceiver::addAction(const QJniObject &action)
{
    if (!valid || !action.isValid())
        return;

    intentFilterObject.callMethod<void>("addAction", action.object<jstring>());

    contextObject.callMethod<QtJniTypes::Intent>(
                "registerReceiver",
                broadcastReceiverObject.object<QtJniTypes::BroadcastReceiver>(),
                intentFilterObject.object<QtJniTypes::IntentFilter>());
}

QT_END_NAMESPACE
