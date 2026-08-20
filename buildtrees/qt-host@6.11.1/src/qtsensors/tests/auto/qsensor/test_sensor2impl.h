// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEST_SENSOR2IMPL_H
#define TEST_SENSOR2IMPL_H

#include <qsensorbackend.h>
#include "test_sensor2.h"

class testsensor2impl : public QSensorBackend
{
public:
    static char const * const id;

    testsensor2impl(QSensor *sensor);

    void start() override;
    void stop() override;

private:
    TestSensor2Reading m_reading;
};

#endif
