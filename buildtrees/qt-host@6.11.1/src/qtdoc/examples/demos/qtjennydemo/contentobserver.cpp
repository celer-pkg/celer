// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "contentobserver.h"
#include <QDebug>

ContentObserver::ContentObserver() {}

void ContentObserver::setHandler(android::os::HandlerProxy handler)
{
    mHandler = handler;
}
jobject ContentObserver::deliverSelfNotifications()
{
    QJniObject res("java/lang/Boolean", true);
    QJniEnvironment env;
    return env->NewGlobalRef(res.object<jobject>());
}

void ContentObserver::onChange(jboolean selfChange)
{
    mHandler.sendEmptyMessage(1);
}

void ContentObserver::onChange(jboolean selfChange, jobject uri)
{
    mHandler.sendEmptyMessage(1);
}

void ContentObserver::onChange__ZLandroid_net_Uri_2I(jboolean selfChange, jobject uri, jint flags)
{
    mHandler.sendEmptyMessage(1);
}

void ContentObserver::onChange__ZLjava_util_Collection_2I(jboolean selfChange, jobject uris, jint flags)
{
    mHandler.sendEmptyMessage(1);
}

