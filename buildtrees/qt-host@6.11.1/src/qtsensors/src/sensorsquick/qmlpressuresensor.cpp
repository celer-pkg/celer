// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "qmlpressuresensor_p.h"
#include <QtSensors/QPressureSensor>

/*!
    \qmltype PressureSensor
//!    \nativetype QmlPressureSensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.1
    \inherits Sensor
    \brief The PressureSensor element reports on atmospheric pressure values.

    The PressureSensor element reports on atmospheric pressure values.

    This element wraps the QPressureSensor class. Please see the documentation for
    QPressureSensor for details.

    \sa PressureReading
*/

QmlPressureSensor::QmlPressureSensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QPressureSensor(this))
{
}

QmlPressureSensor::~QmlPressureSensor()
{
}

QmlSensorReading *QmlPressureSensor::createReading() const
{
    return new QmlPressureReading(m_sensor);
}

QSensor *QmlPressureSensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype PressureReading
//!    \nativetype QmlPressureReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.1
    \inherits SensorReading
    \brief The PressureReading element holds the most recent PressureSensor reading.

    The PressureReading element holds the most recent PressureSensor reading.

    This element wraps the QPressureReading class. Please see the documentation for
    QPressureReading for details.

    This element cannot be directly created.
*/

QmlPressureReading::QmlPressureReading(QPressureSensor *sensor)
    : m_sensor(sensor)
    , m_pressure(0)
    , m_temperature(0)
{
}

QmlPressureReading::~QmlPressureReading()
{
}

/*!
    \qmlproperty real PressureReading::pressure
    This property holds the atmospheric pressure value in Pascals.

    Please see QPressureReading::pressure for information about this property.
*/

qreal QmlPressureReading::pressure() const
{
    return m_pressure;
}

QBindable<qreal> QmlPressureReading::bindablePressure() const
{
    return &m_pressure;
}

/*!
    \qmlproperty real PressureReading::temperature
    This property holds the pressure sensor's temperature value in degrees Celsius.

    Please see QPressureReading::temperature for information about this property.
    \since QtSensors 5.2
*/

qreal QmlPressureReading::temperature() const
{
    return m_temperature;
}

QBindable<qreal> QmlPressureReading::bindableTemperature() const
{
    return &m_temperature;
}

QSensorReading *QmlPressureReading::reading() const
{
    return m_sensor->reading();
}

void QmlPressureReading::readingUpdate()
{
    m_pressure = m_sensor->reading()->pressure();
    m_temperature = m_sensor->reading()->temperature();
}
