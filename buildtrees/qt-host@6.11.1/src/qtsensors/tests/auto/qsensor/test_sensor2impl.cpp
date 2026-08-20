// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "test_sensor2impl.h"
#include <qaccelerometer.h>
#include <QDebug>

char const * const testsensor2impl::id("test sensor 2 impl");

testsensor2impl::testsensor2impl(QSensor *sensor)
    : QSensorBackend(sensor)
{
    setReading<TestSensor2Reading>(&m_reading);
}

void testsensor2impl::start()
{
    QString doThis = sensor()->property("doThis").toString();
    if (doThis == "setOne") {
        m_reading.setTimestamp(1);
        m_reading.setTest(1);
        newReadingAvailable();
    } else {
        m_reading.setTimestamp(2);
        m_reading.setTest(2);
        newReadingAvailable();
    }
}

void testsensor2impl::stop()
{
}

