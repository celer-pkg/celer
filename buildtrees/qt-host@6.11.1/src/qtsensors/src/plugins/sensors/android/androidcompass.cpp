// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// Copyright (C) 2008 The Android Open Source Project
// SPDX-License-Identifier: Apache-2.0

#include "androidcompass.h"

#include <qmath.h>

#include "sensormanager.h"

AndroidCompass::AndroidCompass(QSensor *sensor, QObject *parent)
    : ThreadSafeSensorBackend(sensor, parent)
{
    setDescription("Compass");
    setReading<QCompassReading>(&m_reading);
    memset(&m_accelerometerEvent, 0, sizeof(ASensorVector));
    memset(&m_magneticEvent, 0, sizeof(ASensorVector));
    m_sensorEventQueue = ASensorManager_createEventQueue(m_sensorManager->manager(), m_sensorManager->looper(), -1, &looperCallback, this);
    m_accelerometer = ASensorManager_getDefaultSensor(m_sensorManager->manager(), ASENSOR_TYPE_ACCELEROMETER);
    m_magnetometer = ASensorManager_getDefaultSensor(m_sensorManager->manager(), ASENSOR_TYPE_MAGNETIC_FIELD);
}

AndroidCompass::~AndroidCompass()
{
    stop();
    ASensorManager_destroyEventQueue(m_sensorManager->manager(), m_sensorEventQueue);
}

void AndroidCompass::start()
{
    ASensorEventQueue_enableSensor(m_sensorEventQueue, m_accelerometer);
    if (sensor()->dataRate() > 0)
        ASensorEventQueue_setEventRate(m_sensorEventQueue, m_accelerometer, std::max(ASensor_getMinDelay(m_accelerometer), sensor()->dataRate()));

    ASensorEventQueue_enableSensor(m_sensorEventQueue, m_magnetometer);
    if (sensor()->dataRate() > 0)
        ASensorEventQueue_setEventRate(m_sensorEventQueue, m_magnetometer, std::max(ASensor_getMinDelay(m_magnetometer), sensor()->dataRate()));
}

void AndroidCompass::stop()
{
    ASensorEventQueue_disableSensor(m_sensorEventQueue, m_accelerometer);
    ASensorEventQueue_disableSensor(m_sensorEventQueue, m_magnetometer);
}

void AndroidCompass::readAllEvents()
{
    {
        ASensorEvent sensorEvent;
        QMutexLocker lock(&m_sensorsMutex);
        while (ASensorEventQueue_getEvents(m_sensorEventQueue, &sensorEvent, 1)) {
            switch (sensorEvent.type) {
            case ASENSOR_TYPE_ACCELEROMETER:
                m_accelerometerEvent = sensorEvent.acceleration;
                m_accelerometerEvent.status = m_accelerometerEvent.status == ASENSOR_STATUS_NO_CONTACT ? 0 : m_accelerometerEvent.status;
                break;
            case ASENSOR_TYPE_MAGNETIC_FIELD:
                m_magneticEvent = sensorEvent.magnetic;
                m_magneticEvent.status = m_magneticEvent.status == ASENSOR_STATUS_NO_CONTACT ? 0 : m_magneticEvent.status;
                break;
            }
        }
    }

    QCoreApplication::postEvent(this, new FunctionEvent{[=]() {
        // merged getRotationMatrix https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/hardware/SensorManager.java#1182
        // and getOrientation https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/hardware/SensorManager.java#1477
        QMutexLocker lock(&m_sensorsMutex);
        auto Ax = qreal(m_accelerometerEvent.x);
        auto Ay = qreal(m_accelerometerEvent.y);
        auto Az = qreal(m_accelerometerEvent.z);

        const qreal normsqA = (Ax * Ax + Ay * Ay + Az * Az);
        const auto g = qreal(ASENSOR_STANDARD_GRAVITY);
        const qreal freeFallGravitySquared = 0.01 * g * g;
        if (normsqA < freeFallGravitySquared)
            return;

        auto Ex = qreal(m_magneticEvent.x);
        auto Ey = qreal(m_magneticEvent.y);
        auto Ez = qreal(m_magneticEvent.z);
        qreal Hx = Ey * Az - Ez * Ay;
        qreal Hy = Ez * Ax - Ex * Az;
        qreal Hz = Ex * Ay - Ey * Ax;
        const qreal normH = std::sqrt(Hx * Hx + Hy * Hy + Hz * Hz);

        if (normH < 0.1)
            return;
        const qreal invH = 1.0 / normH;
        Hx *= invH;
        Hy *= invH;
        Hz *= invH;
        const qreal invA = 1.0 / std::sqrt(Ax * Ax + Ay * Ay + Az * Az);
        Ax *= invA;
        Ay *= invA;
        Az *= invA;
        const qreal My = Az * Hx - Ax * Hz;
        qreal azimuth = std::atan2(Hy, My);
        qreal accuracyValue = (m_accelerometerEvent.status + m_magneticEvent.status) / 6.0;
        if (sensor()->skipDuplicates() && qFuzzyCompare(azimuth, m_reading.azimuth()) &&
                qFuzzyCompare(accuracyValue, m_reading.calibrationLevel())) {
            return;
        }
        m_reading.setAzimuth(qRadiansToDegrees(azimuth));
        m_reading.setCalibrationLevel(accuracyValue);
        newReadingAvailable();
    }});
}

int AndroidCompass::looperCallback(int, int, void *data)
{
    auto self = reinterpret_cast<AndroidCompass*>(data);
    self->readAllEvents();
    return 1; // 1 means keep receiving events
}
