// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore/QCoreApplication>
#include <QtCore/QJniObject>
#include <QtCore/QObject>
#include <QtCore/QOperatingSystemVersion>
#include <QtCore/QString>
#include <QtCore/private/qandroidextras_p.h>
#include <QtQml/qqml.h>
#include <QtGui/qguiapplication.h>
#include <qtjenny_output/jenny/proxy/android_app_ActivityProxy.h>
#include <qtjenny_output/jenny/proxy/android_app_BuilderProxy.h>
#include <qtjenny_output/jenny/proxy/android_app_NotificationChannelProxy.h>
#include <qtjenny_output/jenny/proxy/android_app_NotificationManagerProxy.h>
#include <qtjenny_output/jenny/proxy/android_app_NotificationProxy.h>
#include <qtjenny_output/jenny/proxy/android_content_IntentProxy.h>
#include <qtjenny_output/jenny/proxy/android_drawable_drawableProxy.h>
#include <qtjenny_output/jenny/proxy/android_media_AudioManagerProxy.h>
#include <qtjenny_output/jenny/proxy/android_os_ContextProxy.h>
#include <qtjenny_output/jenny/proxy/android_os_PowerManagerProxy.h>
#include <qtjenny_output/jenny/proxy/android_os_VibrationEffectProxy.h>
#include <qtjenny_output/jenny/proxy/android_os_VibratorManagerProxy.h>
#include <qtjenny_output/jenny/proxy/android_os_VibratorProxy.h>
#include <qtjenny_output/jenny/proxy/android_os_WakeLockProxy.h>
#include <qtjenny_output/jenny/proxy/android_provider_GlobalProxy.h>
#include <qtjenny_output/jenny/proxy/android_provider_SettingsProxy.h>
#include <qtjenny_output/jenny/proxy/android_provider_SystemProxy.h>
#include <qtjenny_output/jenny/proxy/android_view_LayoutParamsProxy.h>
#include <qtjenny_output/jenny/proxy/android_view_WindowProxy.h>

class BackEnd : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit BackEnd(QObject *parent = nullptr);

    Q_INVOKABLE void disableFullWakeLock();
    Q_INVOKABLE void disablePartialWakeLock();
    Q_INVOKABLE void notify();
    Q_INVOKABLE void setFullWakeLock();
    Q_INVOKABLE void setPartialWakeLock();
    Q_INVOKABLE void vibrate();

    Q_PROPERTY(bool isFixedVolume READ isFixedVolume CONSTANT)
    Q_PROPERTY(bool canWriteSystemSettings MEMBER m_canWriteSystemSettings NOTIFY canWriteSystemSettingsChanged)
    Q_PROPERTY(int maxVolume READ maxVolume CONSTANT)
    Q_PROPERTY(int minVolume READ minVolume CONSTANT)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)

    android::os::ContextProxy m_context;
    android::provider::SystemProxy m_system;
    android::media::AudioManagerProxy m_audioManager;

    int m_brightness;
    int m_volume;

    bool isFixedVolume() const
    { return m_audioManager.isVolumeFixed(); }

    int maxVolume() const
    { return m_maxVolume; }

    int minVolume() const
    { return m_minVolume; }

    int brightness() const;
    void setBrightness(int newBrightness);

    int volume() const;
    void setVolume(int newVolume);

public slots:
    void onVolumeChangeObserved(int volume);
    void onBrightnessChangeObserved(int brightness);
    void onManageWriteSystemSettings();

signals:
    void showPopup(const QString &message);
    void brightnessChanged();
    void volumeChanged();
    void manageWriteSystemSettings();
    void canWriteSystemSettingsChanged();

private:
    const int m_systemVersion = QOperatingSystemVersion::current().version().majorVersion();

    static constexpr int vibrateTimeInMillisecs = 1000;
    static constexpr double brightnessStep = 1.0 / 255;

    int m_minVolume;
    int m_maxVolume;

    bool m_canWriteSystemSettings = true;

    void createNotification();
    void handleVolumeError(const QString &problem, const QString &solution);

    QNativeInterface::QAndroidApplication *m_qAndroidApp;
    android::app::ActivityProxy m_activityContext;
    android::app::NotificationManagerProxy m_notificationManager;
    android::app::NotificationProxy m_notification;
    android::os::WakeLockProxy m_partialWakeLock;
    android::provider::GlobalProxy m_global;
    android::view::LayoutParamsProxy m_layoutParams;
    android::view::WindowProxy m_window;
};
#endif // BACKEND_H
