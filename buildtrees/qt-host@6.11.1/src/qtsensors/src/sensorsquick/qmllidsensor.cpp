// Copyright (C) 2016 Canonical, Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "qmllidsensor_p.h"
#include <QtSensors/QLidSensor>

/*!
    \qmltype LidSensor
//!    \nativetype QmlLidSensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.9
    \inherits Sensor
    \brief The LidSensor element reports on whether a device is closed.
    \internal

    The LidSensor element reports on whether a device is closed.

    This element wraps the QLidSensor class. Please see the documentation for
    QLidSensor for details.

    \sa LidReading
*/

QmlLidSensor::QmlLidSensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QLidSensor(this))
{
}

QmlLidSensor::~QmlLidSensor()
{
}

QmlSensorReading *QmlLidSensor::createReading() const
{
    return new QmlLidReading(m_sensor);
}

QSensor *QmlLidSensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype LidReading
//!    \nativetype QmlLidReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.9
    \inherits SensorReading
    \brief The LidReading element holds the most recent LidSensor reading.
    \internal

    The LidReading element holds the most recent LidSensor reading.

    This element wraps the QLidReading class. Please see the documentation for
    QLidReading for details.

    This element cannot be directly created.
*/

QmlLidReading::QmlLidReading(QLidSensor *sensor)
    : m_sensor(sensor)
    , m_backClosed(false)
    , m_frontClosed(true)
{
}

QmlLidReading::~QmlLidReading()
{
}

/*!
    \qmlproperty real LidReading::backLidClosed
    This property holds whether the back lid is closed.

    Please see QLidReading::backLidClosed for information about this property.
*/

bool QmlLidReading::backLidClosed() const
{
    return m_backClosed;
}

QBindable<bool> QmlLidReading::bindableBackLidClosed() const
{
    return &m_backClosed;
}

/*!
    \qmlproperty real LidReading::frontLidClosed
    This property holds whether the front lid is closed.

    Please see QLidReading::frontLidClosed for information about this property.
*/

bool QmlLidReading::frontLidClosed() const
{
    return m_frontClosed;
}

QBindable<bool> QmlLidReading::bindableFrontLidClosed() const
{
    return &m_frontClosed;
}

QSensorReading *QmlLidReading::reading() const
{
    return m_sensor->reading();
}

void QmlLidReading::readingUpdate()
{
    m_backClosed = m_sensor->reading()->backLidClosed();
    m_frontClosed = m_sensor->reading()->frontLidClosed();
}
