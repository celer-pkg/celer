// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlproximitysensor_p.h"
#include <QtSensors/QProximitySensor>

/*!
    \qmltype ProximitySensor
//!    \nativetype QmlProximitySensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The ProximitySensor element reports on object proximity.

    The ProximitySensor element reports on object proximity.

    This element wraps the QProximitySensor class. Please see the documentation for
    QProximitySensor for details.

    \sa ProximityReading
*/

QmlProximitySensor::QmlProximitySensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QProximitySensor(this))
{
}

QmlProximitySensor::~QmlProximitySensor()
{
}

QmlSensorReading *QmlProximitySensor::createReading() const
{
    return new QmlProximitySensorReading(m_sensor);
}

QSensor *QmlProximitySensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype ProximityReading
//!    \nativetype QmlProximitySensorReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The ProximityReading element holds the most recent ProximitySensor reading.

    The ProximityReading element holds the most recent ProximitySensor reading.

    This element wraps the QProximityReading class. Please see the documentation for
    QProximityReading for details.

    This element cannot be directly created.
*/

QmlProximitySensorReading::QmlProximitySensorReading(QProximitySensor *sensor)
    : m_sensor(sensor)
{
}

QmlProximitySensorReading::~QmlProximitySensorReading()
{
}

/*!
    \qmlproperty bool ProximityReading::near
    This property holds a value indicating if something is near.

    Please see QProximityReading::near for information about this property.
*/

bool QmlProximitySensorReading::near() const
{
    return m_near;
}

QBindable<bool> QmlProximitySensorReading::bindableNear() const
{
    return &m_near;
}

QSensorReading *QmlProximitySensorReading::reading() const
{
    return m_sensor->reading();
}

void QmlProximitySensorReading::readingUpdate()
{
    m_near = m_sensor->reading()->close();
}
