// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "test_sensor.h"
#include "test_sensor_p.h"

IMPLEMENT_READING(TestSensorReading)

int TestSensorReading::test() const
{
    return d->test;
}

void TestSensorReading::setTest(int test)
{
    d->test = test;
}

// =====================================================================

const char *TestSensor::sensorType("test sensor");

#include "moc_test_sensor.cpp"
