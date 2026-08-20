// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmltiltsensor_p.h"
#include <QtSensors/qtiltsensor.h>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

/*!
    \qmltype TiltSensor
//!    \nativetype QmlTiltSensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The TiltSensor element reports tilt events
           along the X and Y axes.

    The TiltSensor element reports tilt events along the X and Y axes.

    This element wraps the QTiltSensor class. Please see the documentation for
    QTiltSensor for details.

    \sa TiltReading
*/

QmlTiltSensor::QmlTiltSensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QTiltSensor(this))
{
}

QmlTiltSensor::~QmlTiltSensor()
{
}

QmlSensorReading *QmlTiltSensor::createReading() const
{
    return new QmlTiltSensorReading(m_sensor);
}

QSensor *QmlTiltSensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmlmethod void TiltSensor::calibrate()
    Calibrate the tilt sensor.

    \sa QTiltSensor::calibrate()
*/
void QmlTiltSensor::calibrate()
{
    m_sensor->calibrate();
}

/*!
    \qmltype TiltReading
//!    \nativetype QmlTiltSensorReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The TiltReading element holds the most recent TiltSensor reading.

    The TiltReading element holds the most recent TiltSensor reading.

    This element wraps the QTiltReading class. Please see the documentation for
    QTiltReading for details.

    This element cannot be directly created.
*/

QmlTiltSensorReading::QmlTiltSensorReading(QTiltSensor *sensor)
    : m_sensor(sensor)
{
}

QmlTiltSensorReading::~QmlTiltSensorReading()
{
}

/*!
    \qmlproperty real TiltReading::yRotation
    This property holds the amount of tilt on the Y axis.

    Please see QTiltReading::yRotation for information about this property.
*/

qreal QmlTiltSensorReading::yRotation() const
{
    return m_yRotation;
}

QBindable<qreal> QmlTiltSensorReading::bindableYRotation() const
{
    return &m_yRotation;
}

/*!
    \qmlproperty real TiltReading::xRotation
    This property holds the amount of tilt on the X axis.

    Please see QTiltReading::xRotation for information about this property.
*/

qreal QmlTiltSensorReading::xRotation() const
{
    return m_xRotation;
}

QBindable<qreal> QmlTiltSensorReading::bindableXRotation() const
{
    return &m_xRotation;
}

QSensorReading *QmlTiltSensorReading::reading() const
{
    return m_sensor->reading();
}

void QmlTiltSensorReading::readingUpdate()
{
    m_yRotation = m_sensor->reading()->yRotation();
    m_xRotation = m_sensor->reading()->xRotation();
}
