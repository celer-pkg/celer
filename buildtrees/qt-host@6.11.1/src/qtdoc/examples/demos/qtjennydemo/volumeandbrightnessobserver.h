// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef VOLUMEANDBRIGHTNESSOBSERVER_H
#define VOLUMEANDBRIGHTNESSOBSERVER_H
#include <qtjenny_output/jenny/proxy/android_os_CallbackProxy.h>
#include <qtjenny_output/jenny/proxy/android_media_AudioManagerProxy.h>
#include <qtjenny_output/jenny/proxy/android_os_HandlerProxy.h>
#include "qtjenny_output/jenny/proxy/android_os_ContextProxy.h"
#include "qtjenny_output/jenny/proxy/android_content_ContentResolverProxy.h"
#include <qtjenny_output/jenny/proxy/android_provider_SystemProxy.h>
#include "contentobserver.h"
#include "backend.h"

class VolumeAndBrightnessObserver : public QObject, android::os::CallbackInterface
{

    Q_OBJECT

private:
    android::os::ContextProxy m_context;
    android::media::AudioManagerProxy m_audioManager;
    android::os::HandlerProxy m_handler;
    android::content::ContentResolverProxy m_resolver;
    android::provider::SystemProxy m_system;
    QJniObject m_volumeObserver;
    ContentObserver m_contentObserver;
    BackEnd *m_backend;
    int m_lastVolume = -1;
    int m_lastBrightness = -1;

public:
    explicit VolumeAndBrightnessObserver(QObject *parent = nullptr, BackEnd *backend = nullptr);
    jobject handleMessage(jobject arg0) override;
    void start();

signals:
    void brightnessChangeObserved(int brightness);
    void volumeChangeObserved(int volume);
};

#endif // VOLUMEANDBRIGHTNESSOBSERVER_H
