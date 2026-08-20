// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef CONTENTOBSERVER_H
#define CONTENTOBSERVER_H

#include <qtjenny_output/jenny/proxy/android_database_ContentObserverProxy.h>
#include <qtjenny_output/jenny/proxy/android_os_HandlerProxy.h>

class ContentObserver : public android::database::ContentObserverInterface
{
private:
    android::os::HandlerProxy mHandler;
public:
    ContentObserver();
    void setHandler(android::os::HandlerProxy handler);
    virtual jobject deliverSelfNotifications();
    virtual void onChange(jboolean selfChange);
    virtual void onChange(jboolean selfChange, jobject uri);
    virtual void onChange__ZLandroid_net_Uri_2I(jboolean selfChange, jobject uri, jint flags);
    virtual void onChange__ZLjava_util_Collection_2I(jboolean selfChange, jobject uris, jint flags);

};

#endif // CONTENTOBSERVER_H
