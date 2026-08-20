// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QObject>
#include <qaccelerometer.h>
#include <qmagnetometer.h>
#include <qorientationsensor.h>

class MyObject : public QObject
{
    void create();
};

void MyObject::create()
{
//! [Creating a sensor]
// On the heap (deleted when this object is deleted)
QAccelerometer *sensor = new QAccelerometer(this);

// On the stack (deleted when the current scope ends)
QOrientationSensor orient_sensor;
//! [Creating a sensor]

    Q_UNUSED(sensor);
    Q_UNUSED(orient_sensor);

{
//! [2]
QMagnetometer *magnetometer = new QMagnetometer(this);
//! [2]
Q_UNUSED(magnetometer);
}

{
//! [3]
QSensor *magnetometer = new QSensor(QMagnetometer::sensorType, this);
//! [3]
Q_UNUSED(magnetometer);
}

}

