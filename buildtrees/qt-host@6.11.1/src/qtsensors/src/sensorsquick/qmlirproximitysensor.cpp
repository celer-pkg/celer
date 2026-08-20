// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlirproximitysensor_p.h"
#include <QtSensors/QIRProximitySensor>

/*!
    \qmltype IRProximitySensor
//!    \nativetype QmlIRProximitySensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The IRProximitySensor element reports on infra-red reflectance values.
    \internal

    This element wraps the QIRProximitySensor class. Please see the documentation for
    QIRProximitySensor for details.

    \sa IRProximityReading
*/

QmlIRProximitySensor::QmlIRProximitySensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QIRProximitySensor(this))
{
}

QmlIRProximitySensor::~QmlIRProximitySensor()
{
}

QmlSensorReading *QmlIRProximitySensor::createReading() const
{
    return new QmlIRProximitySensorReading(m_sensor);
}

QSensor *QmlIRProximitySensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype IRProximityReading
//!    \nativetype QmlIRProximitySensorReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The IRProximityReading element holds the most recent IR proximity reading.
    \internal

    The IRProximityReading element holds the most recent IR proximity reading.

    This element wraps the QIRProximityReading class. Please see the documentation for
    QIRProximityReading for details.

    This element cannot be directly created.
*/

QmlIRProximitySensorReading::QmlIRProximitySensorReading(QIRProximitySensor *sensor)
    : m_sensor(sensor)
{
}

QmlIRProximitySensorReading::~QmlIRProximitySensorReading()
{
}

/*!
    \qmlproperty real IRProximityReading::reflectance
    This property holds the reflectance value.

    Please see QIRProximityReading::reflectance for information about this property.
*/

qreal QmlIRProximitySensorReading::reflectance() const
{
    return m_reflectance;
}

QBindable<qreal> QmlIRProximitySensorReading::bindableReflectance() const
{
    return &m_reflectance;
}


QSensorReading *QmlIRProximitySensorReading::reading() const
{
    return m_sensor->reading();
}

void QmlIRProximitySensorReading::readingUpdate()
{
    m_reflectance = m_sensor->reading()->reflectance();
}
