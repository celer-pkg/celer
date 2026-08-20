// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlgyroscope_p.h"
#include <QtSensors/QGyroscope>

/*!
    \qmltype Gyroscope
//!    \nativetype QmlGyroscope
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The Gyroscope element reports on rotational acceleration
           around the X, Y and Z axes.

    This element wraps the QGyroscope class. Please see the documentation for
    QGyroscope for details.

    \sa GyroscopeReading
*/

QmlGyroscope::QmlGyroscope(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QGyroscope(this))
{
}

QmlGyroscope::~QmlGyroscope()
{
}

QmlSensorReading *QmlGyroscope::createReading() const
{
    return new QmlGyroscopeReading(m_sensor);
}

QSensor *QmlGyroscope::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype GyroscopeReading
//!    \nativetype QmlGyroscopeReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The GyroscopeReading element holds the most recent Gyroscope reading.

    The GyroscopeReading element holds the most recent Gyroscope reading.

    This element wraps the QGyroscopeReading class. Please see the documentation for
    QGyroscopeReading for details.

    This element cannot be directly created.
*/

QmlGyroscopeReading::QmlGyroscopeReading(QGyroscope *sensor)
    : m_sensor(sensor)
{
}

QmlGyroscopeReading::~QmlGyroscopeReading()
{
}

/*!
    \qmlproperty real GyroscopeReading::x
    This property holds the angular velocity around the x axis.

    Please see QGyroscopeReading::x for information about this property.
*/

qreal QmlGyroscopeReading::x() const
{
    return m_x;
}

QBindable<qreal> QmlGyroscopeReading::bindableX() const
{
    return &m_x;
}

/*!
    \qmlproperty real GyroscopeReading::y
    This property holds the angular velocity around the y axis.

    Please see QGyroscopeReading::y for information about this property.
*/

qreal QmlGyroscopeReading::y() const
{
    return m_y;
}

QBindable<qreal> QmlGyroscopeReading::bindableY() const
{
    return &m_y;
}

/*!
    \qmlproperty real GyroscopeReading::z
    This property holds the angular velocity around the z axis.

    Please see QGyroscopeReading::z for information about this property.
*/

qreal QmlGyroscopeReading::z() const
{
    return m_z;
}

QBindable<qreal> QmlGyroscopeReading::bindableZ() const
{
    return &m_z;
}

QSensorReading *QmlGyroscopeReading::reading() const
{
    return m_sensor->reading();
}

void QmlGyroscopeReading::readingUpdate()
{
    m_x = m_sensor->reading()->x();
    m_y = m_sensor->reading()->y();
    m_z = m_sensor->reading()->z();
}
