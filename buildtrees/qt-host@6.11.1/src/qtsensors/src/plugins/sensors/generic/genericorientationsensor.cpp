// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "genericorientationsensor.h"
#include <QDebug>

char const * const genericorientationsensor::id("generic.orientation");

genericorientationsensor::genericorientationsensor(QSensor *sensor)
    : QSensorBackend(sensor)
{
    accelerometer = new QAccelerometer(this);
    accelerometer->addFilter(this);
    accelerometer->connectToBackend();

    setReading<QOrientationReading>(&m_reading);
    setDataRates(accelerometer);
}

void genericorientationsensor::start()
{
    accelerometer->setDataRate(sensor()->dataRate());
    accelerometer->setAlwaysOn(sensor()->isAlwaysOn());
    accelerometer->start();
    if (!accelerometer->isActive())
        sensorStopped();
    if (accelerometer->isBusy())
        sensorBusy();
}

void genericorientationsensor::stop()
{
    accelerometer->stop();
}

bool genericorientationsensor::filter(QAccelerometerReading *reading)
{
    QOrientationReading::Orientation o = m_reading.orientation();

    if (reading->y() > 7.35)
        o = QOrientationReading::TopUp;
    else if (reading->y() < -7.35)
        o = QOrientationReading::TopDown;
    else if (reading->x() > 7.35)
        o = QOrientationReading::RightUp;
    else if (reading->x() < -7.35)
        o = QOrientationReading::LeftUp;
    else if (reading->z() > 7.35)
        o = QOrientationReading::FaceUp;
    else if (reading->z() < -7.35)
        o = QOrientationReading::FaceDown;

    if (o != m_reading.orientation() || m_reading.timestamp() == 0) {
        m_reading.setTimestamp(reading->timestamp());
        m_reading.setOrientation(o);
        newReadingAvailable();
    }

    return false;
}

