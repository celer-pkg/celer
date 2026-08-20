// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QGuiApplication>
#include <QJniObject>
#include <QDebug>
#include "volumeandbrightnessobserver.h"
#include "qtjenny_output/jenny/proxy/android_provider_SystemProxy.h"
#include "qtjenny_output/jenny/proxy/android_os_LooperProxy.h"

VolumeAndBrightnessObserver::VolumeAndBrightnessObserver(QObject *parent, BackEnd *backend) : QObject(parent), m_backend(backend)
{
    m_context = m_backend->m_context;
    m_audioManager = m_backend->m_audioManager;
    m_system = m_backend->m_system;
    m_handler = android::os::HandlerProxy::newInstance(
        android::os::LooperProxy::getMainLooper().object<jobject>(),
        qt_getProxy("org/qtproject/qtjennydemo/NativeInvocationHandler").object<jobject>());
    m_resolver = m_context.getContentResolver();
    m_lastBrightness = m_backend->m_brightness;
    m_lastVolume = m_backend->m_volume;
}

void VolumeAndBrightnessObserver::start()
{
    m_contentObserver.setHandler(m_handler);
    jlong nativeHandle = reinterpret_cast<jlong>(static_cast<NativeInterface*>(&m_contentObserver));
    QJniObject handler("org/qtproject/qtjennydemo/NativeInvocationHandler", nativeHandle);

    m_volumeObserver = QJniObject("org/qtproject/qtjennydemo/ContentObserverProxy",
                                 "(Landroid/os/Handler;Lorg/qtproject/qtjennydemo/NativeInvocationHandler;)V",
                                 m_handler->object<jobject>(),
                                 handler.object<jobject>());
    m_resolver.registerContentObserver(
        m_system.getCONTENT_URI().object<jobject>(), true, m_volumeObserver.object<jobject>());
}

jobject VolumeAndBrightnessObserver::handleMessage(jobject arg0)
{
    int currentMusicVolume = m_audioManager.getStreamVolume(m_audioManager.STREAM_MUSIC);

    if (currentMusicVolume != m_lastVolume) {
        m_lastVolume = currentMusicVolume;
        emit volumeChangeObserved(currentMusicVolume);
    }

    int currentBrightness = m_system.getInt(
        m_resolver->object<jobject>(),
        QJniObject::fromString(m_system.SCREEN_BRIGHTNESS).object<jstring>(),
        0 // default value if not found
        );

    if (currentBrightness != m_lastBrightness) {
        m_lastBrightness = currentBrightness;
        emit brightnessChangeObserved(currentBrightness);
    }

    QJniObject res("java/lang/Boolean", true);
    QJniEnvironment env;
    return env->NewGlobalRef(res.object<jobject>());
}
