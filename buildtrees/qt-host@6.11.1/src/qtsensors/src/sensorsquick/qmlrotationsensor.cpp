// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlrotationsensor_p.h"
#include <QtSensors/QRotationSensor>

/*!
    \qmltype RotationSensor
//!    \nativetype QmlRotationSensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The RotationSensor element reports on device rotation
           around the X, Y and Z axes.

    The RotationSensor element reports on device rotation
    around the X, Y and Z axes.

    This element wraps the QRotationSensor class. Please see the documentation for
    QRotationSensor for details.

    \sa RotationReading
*/

QmlRotationSensor::QmlRotationSensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QRotationSensor(this))
{
    connect(m_sensor, SIGNAL(hasZChanged(bool)), this, SIGNAL(hasZChanged(bool)));
}

QmlRotationSensor::~QmlRotationSensor()
{
}

QmlSensorReading *QmlRotationSensor::createReading() const
{
    return new QmlRotationSensorReading(m_sensor);
}

QSensor *QmlRotationSensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmlproperty bool RotationSensor::hasZ
    This property holds a value indicating if the z angle is available.

    Please see QRotationSensor::hasZ for information about this property.
*/

bool QmlRotationSensor::hasZ() const
{
    return m_sensor->hasZ();
}

/*!
    \qmltype RotationReading
//!    \nativetype QmlRotationSensorReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The RotationReading element holds the most recent RotationSensor reading.

    The RotationReading element holds the most recent RotationSensor reading.

    This element wraps the QRotationReading class. Please see the documentation for
    QRotationReading for details.

    This element cannot be directly created.
*/

QmlRotationSensorReading::QmlRotationSensorReading(QRotationSensor *sensor)
    : m_sensor(sensor)
{
}

QmlRotationSensorReading::~QmlRotationSensorReading()
{
}

/*!
    \qmlproperty real RotationReading::x
    This property holds the rotation around the x axis.

    Please see QRotationReading::x for information about this property.
*/

qreal QmlRotationSensorReading::x() const
{
    return m_x;
}

QBindable<qreal> QmlRotationSensorReading::bindableX() const
{
    return &m_x;
}

/*!
    \qmlproperty real RotationReading::y
    This property holds the rotation around the y axis.

    Please see QRotationReading::y for information about this property.
*/

qreal QmlRotationSensorReading::y() const
{
    return m_y;
}

QBindable<qreal> QmlRotationSensorReading::bindableY() const
{
    return &m_y;
}

/*!
    \qmlproperty real RotationReading::z
    This property holds the rotation around the z axis.

    Please see QRotationReading::z for information about this property.
*/

qreal QmlRotationSensorReading::z() const
{
    return m_z;
}

QBindable<qreal> QmlRotationSensorReading::bindableZ() const
{
    return &m_z;
}

QSensorReading *QmlRotationSensorReading::reading() const
{
    return m_sensor->reading();
}

void QmlRotationSensorReading::readingUpdate()
{
    m_x = m_sensor->reading()->x();
    m_y = m_sensor->reading()->y();
    m_z = m_sensor->reading()->z();
}
