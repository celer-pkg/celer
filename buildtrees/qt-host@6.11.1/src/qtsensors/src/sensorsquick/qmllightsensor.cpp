// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmllightsensor_p.h"
#include <QtSensors/QLightSensor>

/*!
    \qmltype LightSensor
//!    \nativetype QmlLightSensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The LightSensor element reports on light levels using LUX.

    The LightSensor element reports on light levels using LUX.

    This element wraps the QLightSensor class. Please see the documentation for
    QLightSensor for details.

    \sa LightReading
*/

QmlLightSensor::QmlLightSensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QLightSensor(this))
{
    connect(m_sensor, SIGNAL(fieldOfViewChanged(qreal)),
            this, SIGNAL(fieldOfViewChanged(qreal)));
}

QmlLightSensor::~QmlLightSensor()
{
}

QmlSensorReading *QmlLightSensor::createReading() const
{
    return new QmlLightSensorReading(m_sensor);
}

QSensor *QmlLightSensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmlproperty real LightSensor::fieldOfView
    This property holds a value indicating the field of view.

    Please see QLightSensor::fieldOfView for information about this property.
*/

qreal QmlLightSensor::fieldOfView() const
{
    return m_sensor->fieldOfView();
}

/*!
    \qmltype LightReading
//!    \nativetype QmlLightSensorReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The LightReading element holds the most recent LightSensor reading.

    The LightReading element holds the most recent LightSensor reading.

    This element wraps the QLightReading class. Please see the documentation for
    QLightReading for details.

    This element cannot be directly created.
*/

QmlLightSensorReading::QmlLightSensorReading(QLightSensor *sensor)
    : m_sensor(sensor)
{
}

QmlLightSensorReading::~QmlLightSensorReading()
{
}

/*!
    \qmlproperty real LightReading::illuminance
    This property holds the light level.

    Please see QLightReading::illuminance for information about this property.
*/

qreal QmlLightSensorReading::illuminance() const
{
    return m_illuminance;
}

QBindable<qreal> QmlLightSensorReading::bindableIlluminance() const
{
    return &m_illuminance;
}

QSensorReading *QmlLightSensorReading::reading() const
{
    return m_sensor->reading();
}

void QmlLightSensorReading::readingUpdate()
{
    m_illuminance = m_sensor->reading()->lux();
}
