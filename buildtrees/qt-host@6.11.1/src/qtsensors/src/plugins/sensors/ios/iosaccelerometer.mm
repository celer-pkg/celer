// Copyright (C) 2016 Lorn Potter
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iosaccelerometer.h"
#include "iosmotionmanager.h"

#import <CoreMotion/CoreMotion.h>

char const * const IOSAccelerometer::id("ios.accelerometer");

QT_BEGIN_NAMESPACE

int IOSAccelerometer::s_startCount = 0;

IOSAccelerometer::IOSAccelerometer(QSensor *sensor)
    : QSensorBackend(sensor)
    , m_motionManager([QIOSMotionManager sharedManager])
    , m_timer(0)
{
    setReading<QAccelerometerReading>(&m_reading);
    addDataRate(1, 100); // 100Hz
    addOutputRange(-22.418, 22.418, 0.17651); // 2G
}

void IOSAccelerometer::start()
{
    if (m_timer != 0)
        return;

    int hz = sensor()->dataRate();
    m_timer = startTimer(1000 / (hz == 0 ? 60 : hz));
    if (++s_startCount == 1)
        [m_motionManager startAccelerometerUpdates];
}

void IOSAccelerometer::stop()
{
    if (m_timer == 0)
        return;

    killTimer(m_timer);
    m_timer = 0;
    if (--s_startCount == 0)
        [m_motionManager stopAccelerometerUpdates];
}

void IOSAccelerometer::timerEvent(QTimerEvent *)
{
    // Convert from NSTimeInterval to microseconds and G to m/s2, and flip axes:
    CMAccelerometerData *data = m_motionManager.accelerometerData;
    CMAcceleration acc = data.acceleration;
    // skip update if NaN
    if (acc.x != acc.x || acc.y != acc.y || acc.z != acc.z)
        return;
    static const qreal G = 9.8066;
    m_reading.setTimestamp(quint64(data.timestamp * 1e6));
    m_reading.setX(qreal(acc.x) * G * -1);
    m_reading.setY(qreal(acc.y) * G * -1);
    m_reading.setZ(qreal(acc.z) * G * -1);
    newReadingAvailable();
}

QT_END_NAMESPACE
