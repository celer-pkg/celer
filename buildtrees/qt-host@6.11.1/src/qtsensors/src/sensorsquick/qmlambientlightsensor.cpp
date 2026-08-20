// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlambientlightsensor_p.h"
#include <QtSensors/QAmbientLightSensor>

/*!
    \qmltype AmbientLightSensor
//!    \nativetype QmlAmbientLightSensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The AmbientLightSensor element repors on ambient lighting conditions.

    The AmbientLightSensor element repors on ambient lighting conditions.

    This element wraps the QAmbientLightSensor class. Please see the documentation for
    QAmbientLightSensor for details.

    \sa AmbientLightReading
*/

QmlAmbientLightSensor::QmlAmbientLightSensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QAmbientLightSensor(this))
{
}

QmlAmbientLightSensor::~QmlAmbientLightSensor()
{
}

QmlSensorReading *QmlAmbientLightSensor::createReading() const
{
    return new QmlAmbientLightSensorReading(m_sensor);
}

QSensor *QmlAmbientLightSensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype AmbientLightReading
//!    \nativetype QmlAmbientLightSensorReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The AmbientLightReading element holds the most AmbientLightSensor reading.

    The AmbientLightReading element holds the most AmbientLightSensor reading.

    This element wraps the QAmbientLightReading class. Please see the documentation for
    QAmbientLightReading for details.

    This element cannot be directly created.
*/

QmlAmbientLightSensorReading::QmlAmbientLightSensorReading(QAmbientLightSensor *sensor)
    : m_sensor(sensor)
{
}

QmlAmbientLightSensorReading::~QmlAmbientLightSensorReading()
{
}

/*!
    \qmlproperty LightLevel AmbientLightReading::lightLevel
    This property holds the ambient light level.

    Please see QAmbientLightReading::lightLevel for information about this property.

    Note that LightLevel constants are exposed through the AmbientLightReading class.
    \code
        AmbientLightSensor {
            onReadingChanged: {
                if (reading.lightLevel == AmbientLightReading.Dark)
                    // do something
            }
        }
    \endcode
*/

QAmbientLightReading::LightLevel QmlAmbientLightSensorReading::lightLevel() const
{
    return m_lightLevel;
}

QBindable<QAmbientLightReading::LightLevel> QmlAmbientLightSensorReading::bindableLightLevel() const
{
    return &m_lightLevel;
}

QSensorReading *QmlAmbientLightSensorReading::reading() const
{
    return m_sensor->reading();
}

void QmlAmbientLightSensorReading::readingUpdate()
{
    m_lightLevel = m_sensor->reading()->lightLevel();
}
