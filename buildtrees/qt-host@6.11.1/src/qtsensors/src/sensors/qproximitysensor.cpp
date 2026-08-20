// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qproximitysensor.h"
#include "qproximitysensor_p.h"

QT_BEGIN_NAMESPACE

IMPLEMENT_READING(QProximityReading)

/*!
    \class QProximityReading
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 5.1

    \brief The QProximityReading class represents one reading from the
           proximity sensor.

    \target QProximityReading_Units
    The proximity sensor can only indicate if an object is close or not.

    The distance at which an object is considered close is device-specific. This
    distance may be available in the QSensor::outputRanges property.
*/

/*!
    \property QProximityReading::close
    \brief a value indicating if something is close.

    Set to true if something is close.
    Set to false is nothing is close.

    \sa QProximityReading_Units
*/

bool QProximityReading::close() const
{
    return d->close;
}

/*!
    Sets the close value to \a close.
*/
void QProximityReading::setClose(bool close)
{
    d->close = close;
}

// =====================================================================

/*!
    \class QProximityFilter
    \ingroup sensors_filter
    \inmodule QtSensors
    \since 5.1

    \brief The QProximityFilter class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to QProximityReading
    instead of QSensorReading.
*/

/*!
    \fn QProximityFilter::filter(QProximityReading *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
*/

bool QProximityFilter::filter(QSensorReading *reading)
{
    return filter(static_cast<QProximityReading*>(reading));
}

char const * const QProximitySensor::sensorType("QProximitySensor");

/*!
    \class QProximitySensor
    \ingroup sensors_type
    \inmodule QtSensors
    \since 5.1

    \brief The QProximitySensor class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a QProximityReading instead of a QSensorReading.

    For details about how the sensor works, see \l QProximityReading.

    \sa QProximityReading
*/

/*!
    Construct the sensor as a child of \a parent.
*/
QProximitySensor::QProximitySensor(QObject *parent)
    : QSensor(QProximitySensor::sensorType, parent)
{
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
*/
QProximitySensor::~QProximitySensor()
{
}

/*!
    \fn QProximitySensor::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
*/

QProximityReading *QProximitySensor::reading() const
{
    return static_cast<QProximityReading*>(QSensor::reading());
}

QT_END_NAMESPACE

#include "moc_qproximitysensor.cpp"
