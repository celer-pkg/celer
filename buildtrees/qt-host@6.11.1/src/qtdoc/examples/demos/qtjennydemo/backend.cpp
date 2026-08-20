// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "backend.h"
#include "volumeandbrightnessobserver.h"

BackEnd::BackEnd(QObject *parent) : QObject{ parent }
{
    using namespace android::os;
    using namespace android::app;

    // Initialize members
    m_qAndroidApp = qGuiApp->nativeInterface<QNativeInterface::QAndroidApplication>();
    m_context = ContextProxy(m_qAndroidApp->context());
    m_activityContext = ActivityProxy(m_qAndroidApp->context());
    PowerManagerProxy powerManager = m_activityContext.getSystemService(
        QJniObject::fromString(m_context.POWER_SERVICE).object<jstring>());
    m_audioManager = m_activityContext.getSystemService(
        QJniObject::fromString(m_context.AUDIO_SERVICE).object<jstring>());
    m_window = m_activityContext.getWindow();
    m_layoutParams = m_window.getAttributes();
    m_partialWakeLock = powerManager.newWakeLock(powerManager.PARTIAL_WAKE_LOCK,
        QJniObject::fromString("PARTIALWAKELOCK").object<jstring>());
    m_brightness = m_system.getInt(m_context.getContentResolver().object<jobject>(),
                                   QJniObject::fromString(m_system.SCREEN_BRIGHTNESS).object<jstring>());
    m_maxVolume = m_audioManager.getStreamMaxVolume(m_audioManager.STREAM_MUSIC);
    m_minVolume = m_audioManager.getStreamMinVolume(m_audioManager.STREAM_MUSIC);
    m_volume = m_audioManager.getStreamVolume(m_audioManager.STREAM_MUSIC);

    // Suggests an audio stream whose volume should be changed by the
    // hardware volume controls when this activity is in foreground.
    // With this we can focus on only the music stream, which makes adjusting volume much easier.
    m_activityContext.setVolumeControlStream(m_audioManager.STREAM_MUSIC);

    // If system implements a fixed volume policy, show popup with error message.
    if (m_audioManager.isVolumeFixed())
        handleVolumeError("Device implements fixed volume setting.", "");

    auto vobs = new VolumeAndBrightnessObserver(this, this);

    connect(vobs, &VolumeAndBrightnessObserver::volumeChangeObserved, this, &BackEnd::onVolumeChangeObserved);
    connect(vobs, &VolumeAndBrightnessObserver::brightnessChangeObserved, this, &BackEnd::onBrightnessChangeObserved);
    connect(this, &BackEnd::manageWriteSystemSettings, this, &BackEnd::onManageWriteSystemSettings);

    vobs->start();

    createNotification();
}

void BackEnd::vibrate()
{
    using namespace android::os;

    VibrationEffectProxy effect;
    VibratorManagerProxy vibratorManager;
    VibratorProxy vibrator;

    if (m_systemVersion >= 12) {
        vibratorManager = m_activityContext.getSystemService(
            QJniObject::fromString(m_context.VIBRATOR_MANAGER_SERVICE).object<jstring>());
        vibrator = vibratorManager.getDefaultVibrator();
    } else {
        vibrator = m_activityContext.getSystemService(
            QJniObject::fromString(m_context.VIBRATOR_SERVICE).object<jstring>());
    }

    effect = VibrationEffectProxy().createOneShot(
        vibrateTimeInMillisecs, VibrationEffectProxy().DEFAULT_AMPLITUDE);
    vibrator.vibrate(effect->object<jobject>());
}

// Posts an Android notification
void BackEnd::notify()
{
    using namespace QtAndroidPrivate;

    PermissionResult result = checkPermission("android.permission.POST_NOTIFICATIONS").result();

    if (result == Authorized || m_systemVersion <= 12) {
        m_notificationManager.notify(0, m_notification);
    } else {
        requestPermission("android.permission.POST_NOTIFICATIONS").then(
            [&](PermissionResult result) {
                if (result == Denied)
                    qWarning() << "POST_NOTIFICATIONS permission was denied";
                else if (result == Authorized)
                    m_notificationManager.notify(0, m_notification);
            });
    }
}

void BackEnd::setPartialWakeLock()
{
    m_partialWakeLock.acquire(m_activityContext);
}

void BackEnd::disablePartialWakeLock()
{
    m_partialWakeLock.release(m_activityContext);
}

void BackEnd::setFullWakeLock()
{
    m_qAndroidApp->runOnAndroidMainThread([&]() {
        m_window.addFlags(m_layoutParams.FLAG_KEEP_SCREEN_ON);
    }).then([]() {
            qInfo() << "Full WakeLock set";
        });
}

void BackEnd::disableFullWakeLock()
{
    m_qAndroidApp->runOnAndroidMainThread([&]() {
        m_window.clearFlags(m_layoutParams.FLAG_KEEP_SCREEN_ON);
    }).then([]() {
            qInfo() << "Full WakeLock released";
        });
}

// Creates a notification that is later posted in notify() method
void BackEnd::createNotification()
{
    using namespace android::app;
    using namespace android::drawable;

    NotificationChannelProxy channel;
    BuilderProxy builder;

    channel = NotificationChannelProxy().newInstance(
        QJniObject::fromString("01").object<jstring>(),
        QJniObject::fromString("QtJenny").object<jstring>(),
        m_notificationManager.IMPORTANCE_HIGH);

    m_notificationManager = m_activityContext.getSystemService(
        QJniObject::fromString(m_context.NOTIFICATION_SERVICE).object<jstring>());
    m_notificationManager.createNotificationChannel(channel);

    builder = BuilderProxy().newInstance(m_context,
        QJniObject::fromString(channel.getId().toString()).object<jstring>());
    builder.setSmallIcon(drawableProxy::ic_dialog_info);
    builder.setContentTitle(QJniObject::fromString("QtJenny").object<jstring>());
    builder.setContentText(QJniObject::fromString(
                                 "Hello from QtJenny app!").object<jstring>());
    builder.setDefaults(m_notification.DEFAULT_SOUND);
    builder.setAutoCancel(true);
    m_notification = builder.build();
}

void BackEnd::handleVolumeError(const QString &problem, const QString &solution)
{
    if (!problem.isEmpty())
        qWarning() << problem;

    const QString message = problem.isEmpty() ? solution
                                              : problem + "\n" + solution;
    emit showPopup(message);
}

int BackEnd::brightness() const
{
    return m_brightness;
}

void BackEnd::setBrightness(int newBrightness)
{

    if (m_brightness == newBrightness)
        return;

    // Check if the app has permission to write system settings.
    if (!m_system.canWrite(m_context)) {
        m_canWriteSystemSettings = false;
        emit showPopup("Writing system settings is not allowed."
                       " Please give permission to write system settings for this application");
        return;
    }

    m_canWriteSystemSettings = true;

    m_brightness = newBrightness;

    double brightnessToDouble = brightnessStep * newBrightness;

    // This keeps the brightness shown in the system bar of Android device in sync.
    m_system.putInt(m_context.getContentResolver().object<jobject>(),
                    QJniObject::fromString(m_system.SCREEN_BRIGHTNESS).object<jstring>(),
                    newBrightness);
    m_qAndroidApp->runOnAndroidMainThread([this, brightnessToDouble]() {
        m_layoutParams.setScreenBrightness(brightnessToDouble);
        m_window.setAttributes(m_layoutParams);
    });
}

int BackEnd::volume() const
{
    return m_volume;
}

void BackEnd::setVolume(int newVolume)
{
    if (m_volume == newVolume)
        return;
    m_audioManager.setStreamVolume(m_audioManager.STREAM_MUSIC, newVolume, m_audioManager.FLAG_SHOW_UI);
    m_volume = newVolume;
}

void BackEnd::onVolumeChangeObserved(int volume)
{
    if (m_volume == volume)
        return;
    m_volume = volume;
    emit volumeChanged();
}

void BackEnd::onBrightnessChangeObserved(int brightness)
{
    if (m_brightness == brightness)
        return;
    m_brightness = brightness;
    emit brightnessChanged();
}

void BackEnd::onManageWriteSystemSettings()
{
    // Start an Activity with the ACTION_MANAGE_WRITE_SETTINGS intent if app
    // does not have permission to write system settings, after which user
    // has to manually give the permission to write system settings to this app.
    android::content::IntentProxy m_intent = m_intent.newInstance(
        QJniObject::fromString(android::provider::SettingsProxy::ACTION_MANAGE_WRITE_SETTINGS).object<jstring>());
    m_context.startActivity(m_intent);
}
