// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlaccelerometer_p.h"
#include <QtSensors/QAccelerometer>

/*!
    \qmltype Accelerometer
//!    \nativetype QmlAccelerometer
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The Accelerometer element reports on linear acceleration
           along the X, Y and Z axes.

    The Accelerometer element reports on linear acceleration
    along the X, Y and Z axes.

    This element wraps the QAccelerometer class. Please see the documentation for
    QAccelerometer for details.

    \sa AccelerometerReading
*/

QmlAccelerometer::QmlAccelerometer(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QAccelerometer(this))
{
    connect(m_sensor, SIGNAL(accelerationModeChanged(AccelerationMode)),
            this, SIGNAL(accelerationModeChanged(AccelerationMode)));
}

QmlAccelerometer::~QmlAccelerometer()
{
}

/*!
    \qmlproperty AccelerationMode Accelerometer::accelerationMode
    \since QtSensors 5.1

    This property holds the current acceleration mode.

    Please see QAccelerometer::accelerationMode for information about this property.
*/

QmlAccelerometer::AccelerationMode QmlAccelerometer::accelerationMode() const
{
    return static_cast<QmlAccelerometer::AccelerationMode>(m_sensor->accelerationMode());
}

void QmlAccelerometer::setAccelerationMode(QmlAccelerometer::AccelerationMode accelerationMode)
{
    m_sensor->setAccelerationMode(static_cast<QAccelerometer::AccelerationMode>(accelerationMode));
}

QmlSensorReading *QmlAccelerometer::createReading() const
{
    return new QmlAccelerometerReading(m_sensor);
}

QSensor *QmlAccelerometer::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype AccelerometerReading
//!    \nativetype QmlAccelerometerReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The AccelerometerReading element holds the most recent Accelerometer reading.

    The AccelerometerReading element holds the most recent Accelerometer reading.

    This element wraps the QAccelerometerReading class. Please see the documentation for
    QAccelerometerReading for details.

    This element cannot be directly created.
*/

QmlAccelerometerReading::QmlAccelerometerReading(QAccelerometer *sensor)
    : m_sensor(sensor)
{
}

QmlAccelerometerReading::~QmlAccelerometerReading()
{
}

/*!
    \qmlproperty real AccelerometerReading::x
    This property holds the acceleration on the X axis.

    Please see QAccelerometerReading::x for information about this property.
*/

qreal QmlAccelerometerReading::x() const
{
    return m_x;
}

QBindable<qreal> QmlAccelerometerReading::bindableX() const
{
    return &m_x;
}

/*!
    \qmlproperty real AccelerometerReading::y
    This property holds the acceleration on the Y axis.

    Please see QAccelerometerReading::y for information about this property.
*/

qreal QmlAccelerometerReading::y() const
{
    return m_y;
}

QBindable<qreal> QmlAccelerometerReading::bindableY() const
{
    return &m_y;
}

/*!
    \qmlproperty real AccelerometerReading::z
    This property holds the acceleration on the Z axis.

    Please see QAccelerometerReading::z for information about this property.
*/

qreal QmlAccelerometerReading::z() const
{
    return m_z;
}

QBindable<qreal> QmlAccelerometerReading::bindableZ() const
{
    return &m_z;
}

QSensorReading *QmlAccelerometerReading::reading() const
{
    return m_sensor->reading();
}

void QmlAccelerometerReading::readingUpdate()
{
    m_x = m_sensor->reading()->x();
    m_y = m_sensor->reading()->y();
    m_z = m_sensor->reading()->z();
}
