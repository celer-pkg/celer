// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "qmlambienttemperaturesensor_p.h"
#include <QtSensors/QAmbientTemperatureSensor>

/*!
    \qmltype AmbientTemperatureSensor
//!    \nativetype QmlAmbientTemperatureSensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.1
    \inherits Sensor
    \brief The AmbientTemperatureSensor element reports on the ambient temperature.

    The AmbientTemperatureSensor element reports on the ambient temperature.

    This element wraps the QAmbientTemperatureSensor class. Please see the documentation for
    QAmbientTemperatureSensor for details.

    \sa AmbientTemperatureReading
*/

QmlAmbientTemperatureSensor::QmlAmbientTemperatureSensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QAmbientTemperatureSensor(this))
{
}

QmlAmbientTemperatureSensor::~QmlAmbientTemperatureSensor()
{
}

QmlSensorReading *QmlAmbientTemperatureSensor::createReading() const
{
    return new QmlAmbientTemperatureReading(m_sensor);
}

QSensor *QmlAmbientTemperatureSensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype AmbientTemperatureReading
//!    \nativetype QmlAmbientTemperatureReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.1
    \inherits SensorReading
    \brief The AmbientTemperatureReading element holds the most recent temperature reading.

    The AmbientTemperatureReading element holds the most recent temperature reading.

    This element wraps the QAmbientTemperatureReading class. Please see the documentation for
    QAmbientTemperatureReading for details.

    This element cannot be directly created.
*/

QmlAmbientTemperatureReading::QmlAmbientTemperatureReading(QAmbientTemperatureSensor *sensor)
    : m_sensor(sensor)
    , m_temperature(0)
{
}

QmlAmbientTemperatureReading::~QmlAmbientTemperatureReading()
{
}

/*!
    \qmlproperty real AmbientTemperatureReading::temperature
    This property holds the ambient temperature in degree Celsius.

    Please see QAmbientTemperatureReading::temperature for information about this property.
*/

qreal QmlAmbientTemperatureReading::temperature() const
{
    return m_temperature;
}

QBindable<qreal> QmlAmbientTemperatureReading::bindableTemperature() const
{
    return &m_temperature;
}

QSensorReading *QmlAmbientTemperatureReading::reading() const
{
    return m_sensor->reading();
}

void QmlAmbientTemperatureReading::readingUpdate()
{
    m_temperature = m_sensor->reading()->temperature();
}
