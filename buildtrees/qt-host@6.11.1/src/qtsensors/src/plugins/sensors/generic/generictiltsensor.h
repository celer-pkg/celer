// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GENERICTILTSENSOR_H
#define GENERICTILTSENSOR_H

#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qtiltsensor.h>
#include <QtSensors/qaccelerometer.h>

QT_BEGIN_NAMESPACE

class GenericTiltSensor : public QSensorBackend, public QAccelerometerFilter
{
    Q_OBJECT
public:

    static char const * const id;

    GenericTiltSensor(QSensor *sensor);

    void start() override;
    void stop() override;

    Q_INVOKABLE void calibrate();

    bool filter(QAccelerometerReading *reading) override;

    bool isFeatureSupported(QSensor::Feature feature) const override;

private:
    QTiltReading m_reading;
    QAccelerometer *accelerometer;
    qreal radAccuracy;
    qreal pitch;
    qreal roll;
    qreal calibratedPitch;
    qreal calibratedRoll;
    qreal xRotation;
    qreal yRotation;
};

QT_END_NAMESPACE

#endif

