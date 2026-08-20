// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GENERICROTATIONSENSOR_H
#define GENERICROTATIONSENSOR_H

#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qrotationsensor.h>
#include <QtSensors/qaccelerometer.h>
#include <QtSensors/qmagnetometer.h>

class genericrotationsensor : public QSensorBackend, public QSensorFilter
{
public:
    static char const * const id;

    genericrotationsensor(QSensor *sensor);

    void start() override;
    void stop() override;

    bool filter(QSensorReading *reading) override;

private:
    QRotationReading m_reading;
    QAccelerometer *accelerometer;
};

#endif

