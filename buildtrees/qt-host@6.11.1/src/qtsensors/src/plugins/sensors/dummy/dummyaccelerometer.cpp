// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "dummyaccelerometer.h"
#include <QDebug>
#include <QtGlobal>

char const * const dummyaccelerometer::id("dummy.accelerometer");

dummyaccelerometer::dummyaccelerometer(QSensor *sensor)
    : dummycommon(sensor)
{
    setReading<QAccelerometerReading>(&m_reading);
    addDataRate(100, 100); // 100Hz
}

void dummyaccelerometer::poll()
{
    m_reading.setTimestamp(getTimestamp());
    // Your average desktop computer doesn't move :)
    m_reading.setX(0);
    m_reading.setY(9.8); // facing the user, gravity goes here
    m_reading.setZ(0);

    newReadingAvailable();
}

