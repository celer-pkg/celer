// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iosmotionmanager.h"
#include "iosgyroscope.h"

#import <CoreMotion/CoreMotion.h>
#import <QtCore/qmath.h>

char const * const IOSGyroscope::id("ios.gyroscope");

QT_BEGIN_NAMESPACE

int IOSGyroscope::s_startCount = 0;

IOSGyroscope::IOSGyroscope(QSensor *sensor)
    : QSensorBackend(sensor)
    , m_motionManager([QIOSMotionManager sharedManager])
    , m_timer(0)
{
    setReading<QGyroscopeReading>(&m_reading);
    addDataRate(1, 100); // 100Hz is max it seems
    addOutputRange(-360, 360, 0.01);
}

void IOSGyroscope::start()
{
    if (m_timer != 0)
        return;

    int hz = sensor()->dataRate();
    m_timer = startTimer(1000 / (hz == 0 ? 60 : hz));
    if (++s_startCount == 1)
        [m_motionManager startGyroUpdates];
}

void IOSGyroscope::stop()
{
    if (m_timer == 0)
        return;

    killTimer(m_timer);
    m_timer = 0;
    if (--s_startCount == 0)
        [m_motionManager stopGyroUpdates];
}

void IOSGyroscope::timerEvent(QTimerEvent *)
{
    // Convert NSTimeInterval to microseconds and radians to degrees:
    CMGyroData *data = m_motionManager.gyroData;
    CMRotationRate rate = data.rotationRate;
    // skip update if NaN
    if (rate.x != rate.x || rate.y != rate.y || rate.z != rate.z)
        return;
    m_reading.setTimestamp(quint64(data.timestamp * 1e6));
    m_reading.setX(qRadiansToDegrees(qreal(rate.x)));
    m_reading.setY(qRadiansToDegrees(qreal(rate.y)));
    m_reading.setZ(qRadiansToDegrees(qreal(rate.z)));
    newReadingAvailable();
}

QT_END_NAMESPACE
