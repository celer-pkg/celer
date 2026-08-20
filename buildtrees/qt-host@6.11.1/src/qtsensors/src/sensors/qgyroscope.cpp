// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgyroscope.h"
#include "qgyroscope_p.h"

QT_BEGIN_NAMESPACE

IMPLEMENT_READING(QGyroscopeReading)

/*!
    \class QGyroscopeReading
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 5.1

    \brief The QGyroscopeReading class represents one reading from the
           gyroscope sensor.

    \section2 QGyroscopeReading Units

    The reading contains 3 values, measured in degrees per second that define
    the movement of the device around the x, y and z axes. Unlike QRotationReading,
    the values represent the current angular velocity rather than a fixed rotation.
    The measurements are in degrees per second.

    \image sensors-coordinates3.jpg
*/

/*!
    \property QGyroscopeReading::x
    \brief the angular velocity around the x axis.

    Measured as degrees per second.
    \sa {QGyroscopeReading Units}
*/

qreal QGyroscopeReading::x() const
{
    return d->x;
}

/*!
    Sets the angular velocity around the x axis to \a x.
*/
void QGyroscopeReading::setX(qreal x)
{
    d->x = x;
}

/*!
    \property QGyroscopeReading::y
    \brief the angular velocity around the y axis.

    Measured as degrees per second.
    \sa {QGyroscopeReading Units}
*/

qreal QGyroscopeReading::y() const
{
    return d->y;
}

/*!
    Sets the angular velocity around the y axis to \a y.
*/
void QGyroscopeReading::setY(qreal y)
{
    d->y = y;
}

/*!
    \property QGyroscopeReading::z
    \brief the angular velocity around the z axis.

    Measured as degrees per second.
    \sa {QGyroscopeReading Units}
*/

qreal QGyroscopeReading::z() const
{
    return d->z;
}

/*!
    Sets the angular velocity around the z axis to \a z.
*/
void QGyroscopeReading::setZ(qreal z)
{
    d->z = z;
}

// =====================================================================

/*!
    \class QGyroscopeFilter
    \ingroup sensors_filter
    \inmodule QtSensors
    \since 5.1

    \brief The QGyroscopeFilter class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to QGyroscopeReading
    instead of QSensorReading.
*/

/*!
    \fn QGyroscopeFilter::filter(QGyroscopeReading *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
*/

bool QGyroscopeFilter::filter(QSensorReading *reading)
{
    return filter(static_cast<QGyroscopeReading*>(reading));
}

char const * const QGyroscope::sensorType("QGyroscope");

/*!
    \class QGyroscope
    \ingroup sensors_type
    \inmodule QtSensors
    \since 5.1

    \brief The QGyroscope class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a QGyroscopeReading instead of a QSensorReading.

    For details about how the sensor works, see \l QGyroscopeReading.

    \sa QGyroscopeReading
*/

/*!
    Construct the sensor as a child of \a parent.
*/
QGyroscope::QGyroscope(QObject *parent)
    : QSensor(QGyroscope::sensorType, parent)
{
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
*/
QGyroscope::~QGyroscope()
{
}

/*!
    \fn QGyroscope::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
*/

QGyroscopeReading *QGyroscope::reading() const
{
    return static_cast<QGyroscopeReading*>(QSensor::reading());
}

QT_END_NAMESPACE

#include "moc_qgyroscope.cpp"
