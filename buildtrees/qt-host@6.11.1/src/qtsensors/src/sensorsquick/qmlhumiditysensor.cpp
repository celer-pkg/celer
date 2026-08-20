// Copyright (C) 2016 Canonical Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlhumiditysensor_p.h"
#include <QtSensors/QHumiditySensor>

/*!
    \qmltype HumiditySensor
//!    \nativetype QmlHumiditySensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.9
    \inherits Sensor
    \brief The HumiditySensor element reports on humidity.

    The HumiditySensor element reports on humidity.

    This element wraps the QHumiditySensor class. Please see the documentation for
    QHumiditySensor for details.

    \sa HumidityReading
*/

QmlHumiditySensor::QmlHumiditySensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QHumiditySensor(this))
{
}

QmlHumiditySensor::~QmlHumiditySensor()
{
}

QmlSensorReading *QmlHumiditySensor::createReading() const
{
    return new QmlHumidityReading(m_sensor);
}

QSensor *QmlHumiditySensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype HumidityReading
//!    \nativetype QmlHumidityReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.9
    \inherits SensorReading
    \brief The HumidityReading element holds the most recent HumiditySensor reading.

    The HumidityReading element holds the most recent HumiditySensor reading.

    This element wraps the QHumidityReading class. Please see the documentation for
    QHumidityReading for details.

    This element cannot be directly created.
*/

QmlHumidityReading::QmlHumidityReading(QHumiditySensor *sensor)
    : m_sensor(sensor)
    , m_relativeHumidity(0)
    , m_absoluteHumidity(0)
{
}

QmlHumidityReading::~QmlHumidityReading()
{
}

/*!
    \qmlproperty real HumidityReading::relativeHumidity
    This property holds the relative humidity as a percentage.

    Please see QHumidityReading::relativeHumidity for information about this property.
*/

qreal QmlHumidityReading::relativeHumidity() const
{
    return m_relativeHumidity;
}

QBindable<qreal> QmlHumidityReading::bindableRelativeHumidity() const
{
    return &m_relativeHumidity;
}

/*!
    \qmlproperty real HumidityReading::absoluteHumidity
    This property holds the absolute humidity in grams per cubic meter (g/m3).

    Please see QHumidityReading::absoluteHumidity for information about this property.
*/

qreal QmlHumidityReading::absoluteHumidity() const
{
    return m_absoluteHumidity;
}

QBindable<qreal> QmlHumidityReading::bindableAbsoluteHumidity() const
{
    return &m_absoluteHumidity;
}

QSensorReading *QmlHumidityReading::reading() const
{
    return m_sensor->reading();
}

void QmlHumidityReading::readingUpdate()
{
    m_relativeHumidity = m_sensor->reading()->relativeHumidity();
    m_absoluteHumidity = m_sensor->reading()->absoluteHumidity();
}
