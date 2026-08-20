// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmltapsensor_p.h"
#include <QtSensors/QTapSensor>

/*!
    \qmltype TapSensor
//!    \nativetype QmlTapSensor
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The TapSensor element reports tap and double tap events
           along the X, Y and Z axes.
    \internal

    The TapSensor element reports tap and double tap events
    along the X, Y and Z axes.

    This element wraps the QTapSensor class. Please see the documentation for
    QTapSensor for details.

    \sa TapReading
*/

QmlTapSensor::QmlTapSensor(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QTapSensor(this))
{
    connect(m_sensor, SIGNAL(returnDoubleTapEventsChanged(bool)),
            this, SIGNAL(returnDoubleTapEventsChanged(bool)));
}

QmlTapSensor::~QmlTapSensor()
{
}

QmlSensorReading *QmlTapSensor::createReading() const
{
    return new QmlTapSensorReading(m_sensor);
}

QSensor *QmlTapSensor::sensor() const
{
    return m_sensor;
}

/*!
    \qmlproperty bool TapSensor::returnDoubleTapEvents
    This property holds a value indicating if double tap events should be reported.

    Please see QTapSensor::returnDoubleTapEvents for information about this property.
*/

bool QmlTapSensor::returnDoubleTapEvents() const
{
    return m_sensor->returnDoubleTapEvents();
}

void QmlTapSensor::setReturnDoubleTapEvents(bool ret)
{
    m_sensor->setReturnDoubleTapEvents(ret);
}

/*!
    \qmltype TapReading
//!    \nativetype QmlTapSensorReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The TapReading element holds the most recent TapSensor reading.
    \internal

    The TapReading element holds the most recent TapSensor reading.

    This element wraps the QTapReading class. Please see the documentation for
    QTapReading for details.

    This element cannot be directly created.
*/

QmlTapSensorReading::QmlTapSensorReading(QTapSensor *sensor)
    : m_sensor(sensor)
{
}

QmlTapSensorReading::~QmlTapSensorReading()
{
}

/*!
    \qmlproperty TapDirection TapReading::tapDirection
    This property holds the direction of the tap.

    Please see QTapReading::tapDirection for information about this property.

    Note that TapDirection constants are exposed through the TapReading class.
    \code
        TapSensor {
            onReadingChanged: {
                if ((reading.tapDirection & TapReading.X_Both))
                    // do something
            }
        }
    \endcode
*/

QTapReading::TapDirection QmlTapSensorReading::tapDirection() const
{
    return m_tapDirection;
}

QBindable<QTapReading::TapDirection> QmlTapSensorReading::bindableTapDirection() const
{
    return &m_tapDirection;
}


/*!
    \qmlproperty bool TapReading::doubleTap
    This property holds a value indicating if there was a single or double tap.

    Please see QTapReading::doubleTap for information about this property.
*/

bool QmlTapSensorReading::isDoubleTap() const
{
    return m_isDoubleTap;
}

QBindable<bool> QmlTapSensorReading::bindableDoubleTap() const
{
    return &m_isDoubleTap;
}

QSensorReading *QmlTapSensorReading::reading() const
{
    return const_cast<QTapSensor*>(m_sensor)->reading();
}

void QmlTapSensorReading::readingUpdate()
{
    m_tapDirection = m_sensor->reading()->tapDirection();
    m_isDoubleTap = m_sensor->reading()->isDoubleTap();
}
