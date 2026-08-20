// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEST_SENSORIMPL_H
#define TEST_SENSORIMPL_H

#include <qsensorbackend.h>
#include "test_sensor.h"

class testsensorimpl : public QSensorBackend
{
    Q_OBJECT
public:
    static const char *id;

    testsensorimpl(QSensor *sensor);
    ~testsensorimpl();

    void start() override;
    void stop() override;
    bool isFeatureSupported(QSensor::Feature feature) const override;

signals:
    void emitBusyChanged();

private:
    TestSensorReading m_reading;
};

#endif
