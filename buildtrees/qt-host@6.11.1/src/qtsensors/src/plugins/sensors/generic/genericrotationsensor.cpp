// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "genericrotationsensor.h"
#include <QDebug>
#include <qmath.h>

char const * const genericrotationsensor::id("generic.rotation");

genericrotationsensor::genericrotationsensor(QSensor *sensor)
    : QSensorBackend(sensor)
{
    accelerometer = new QAccelerometer(this);
    accelerometer->addFilter(this);
    accelerometer->connectToBackend();

    setReading<QRotationReading>(&m_reading);
    setDataRates(accelerometer);

    QRotationSensor * const rotationSensor = qobject_cast<QRotationSensor *>(sensor);
    if (rotationSensor)
        rotationSensor->setHasZ(false);
}

void genericrotationsensor::start()
{
    accelerometer->setDataRate(sensor()->dataRate());
    accelerometer->setAlwaysOn(sensor()->isAlwaysOn());
    accelerometer->start();
    if (!accelerometer->isActive())
        sensorStopped();
    if (accelerometer->isBusy())
        sensorBusy();
}

void genericrotationsensor::stop()
{
    accelerometer->stop();
}

bool genericrotationsensor::filter(QSensorReading *reading)
{
    QAccelerometerReading *ar = qobject_cast<QAccelerometerReading*>(reading);
    qreal pitch = 0;
    qreal roll = 0;

    qreal x = ar->x();
    qreal y = ar->y();
    qreal z = ar->z();

    // Note that the formula used come from this document:
    // http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf
    pitch = qRadiansToDegrees(qAtan(y / qSqrt(x * x + z * z)));
    roll  = qRadiansToDegrees(qAtan(x / qSqrt(y * y + z * z)));
    // Roll is a left-handed rotation but we need right-handed rotation
    roll = -roll;

    // We need to fix up roll to the (-180,180] range required.
    // Check for negative theta values and apply an offset as required.
    // Note that theta is defined as the angle of the Z axis relative
    // to gravity (see referenced document). It's negative when the
    // face of the device points downward.
    qreal theta = qRadiansToDegrees(qAtan(qSqrt(x * x + y * y) / z));
    if (theta < 0) {
        if (roll > 0)
            roll = 180 - roll;
        else
            roll = -180 - roll;
    }

    m_reading.setTimestamp(ar->timestamp());
    m_reading.setFromEuler(pitch, roll, 0);
    newReadingAvailable();
    return false;
}

