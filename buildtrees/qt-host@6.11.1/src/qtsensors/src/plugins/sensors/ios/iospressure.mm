// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iospressure.h"
#import <CoreMotion/CoreMotion.h>

char const * const IOSPressure::id("ios.pressure");

QT_BEGIN_NAMESPACE

int IOSPressure::s_startCount = 0;

IOSPressure::IOSPressure(QSensor *sensor)
    : QSensorBackend(sensor)
{
    m_altimeter = [[CMAltimeter alloc] init];
    setReading<QPressureReading>(&m_reading);
    addDataRate(1, 100); // 100Hz
}

IOSPressure::~IOSPressure()
{
    [m_altimeter stopRelativeAltitudeUpdates];
    [m_altimeter release];
}

void IOSPressure::start()
{
    if (m_timer != 0)
        return;

    int hz = sensor()->dataRate();
    m_timer = startTimer(1000 / (hz == 0 ? 60 : hz));
    if (++s_startCount == 1) {
        [m_altimeter startRelativeAltitudeUpdatesToQueue:[NSOperationQueue mainQueue]
            withHandler:^(CMAltitudeData * _Nullable altitudeData , NSError * _Nullable error) {
                if (error == nil) {
                    m_reading.setPressure([altitudeData.pressure doubleValue] * 1000);
                    m_reading.setTimestamp(quint64(altitudeData.timestamp * 1e6));
                }
            }];
    }
}

void IOSPressure::stop()
{
    if (m_timer == 0)
        return;

    killTimer(m_timer);
    m_timer = 0;
    if (--s_startCount == 0)
        [m_altimeter stopRelativeAltitudeUpdates];
}

void IOSPressure::timerEvent(QTimerEvent *)
{
    // skip update if NaN or 0
    if ((m_reading.pressure() != m_reading.pressure()) ||
        m_reading.pressure() == 0)
        return;
    newReadingAvailable();
}

QT_END_NAMESPACE
