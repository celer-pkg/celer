// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDACCELEROMETER_H
#define ANDROIDACCELEROMETER_H

#include <qaccelerometer.h>

#include "sensoreventqueue.h"

class AndroidAccelerometer : public SensorEventQueue<QAccelerometerReading>
{
    Q_OBJECT
public:
    enum AccelerationModes {
        Accelerometer = 1,
        Gravity = 2,
        LinearAcceleration = 4,
        AllModes = (Accelerometer | Gravity | LinearAcceleration)
    };
public:
    AndroidAccelerometer(int accelerationModes, QSensor *sensor, QObject *parent = nullptr);
    // QSensorBackend interface
    bool isFeatureSupported(QSensor::Feature feature) const override;

protected:
    // SensorEventQueue interface
    void dataReceived(const ASensorEvent &event) override;

private:
    void applyAccelerationMode(QAccelerometer::AccelerationMode accelerationMode);

private:
    int m_accelerationModes;

};

#endif // ANDROIDACCELEROMETER_H
