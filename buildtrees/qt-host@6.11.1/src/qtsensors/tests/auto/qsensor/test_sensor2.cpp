// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "test_sensor2.h"
#include "test_sensor2_p.h"

IMPLEMENT_READING(TestSensor2Reading)

int TestSensor2Reading::test() const
{
    return d->test;
}

void TestSensor2Reading::setTest(int test)
{
    d->test = test;
}

// =====================================================================

char const * const TestSensor2::sensorType("test sensor 2");

#include "moc_test_sensor2.cpp"
