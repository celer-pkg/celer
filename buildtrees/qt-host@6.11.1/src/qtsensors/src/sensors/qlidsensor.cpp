// Copyright (C) 2016 Canonical, Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include <qlidsensor.h>
#include "qlidsensor_p.h"

QT_BEGIN_NAMESPACE

IMPLEMENT_READING(QLidReading)

/*!
    \class QLidReading
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 5.9
    \internal

    \brief The QLidReading class holds readings from the Lid sensor.

    A normal laptop has what we call a front lid.

    If the laptop can be converted to a tablet by rotating or closing the lid
    where the display is out, this is known as a back lid.

    \section2 QLidReading Units
    The Lid sensor can detect if a device's lid is closed or not. A lid can be a laptop,
    a laptop that converts to a tablet, or even a cover for a tablet or phone.
*/

/*!
    \property QLidReading::backLidClosed
    \brief A value indicating whether the back lid is closed.
    A back lid can be when a convertable laptop is closed
    into to tablet mode without keyboard.

    \sa {QLidReading Units}
*/

bool QLidReading::backLidClosed() const
{
    return d->backLidClosed;
}

/*!
    Sets the backLidClosed value to \a closed.
*/
void QLidReading::setBackLidClosed(bool closed)
{
    d->backLidClosed = closed;
}

/*!
    \property QLidReading::frontLidClosed
    \brief A value indicating whether the front lid is closed.
    A front lid would be a normal laptop lid.
    \sa {QLidReading Units}
*/

bool QLidReading::frontLidClosed() const
{
    return d->frontLidClosed;
}

/*!
    Sets the frontLidClosed value to \a closed.
*/
void QLidReading::setFrontLidClosed(bool closed)
{
    d->frontLidClosed = closed;
}

// =====================================================================

/*!
    \class QLidFilter
    \ingroup sensors_filter
    \inmodule QtSensors
    \since 5.9
    \internal

    \brief The QLidFilter class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to QLidReading
    instead of QSensorReading.
*/

/*!
    \fn QLidFilter::filter(QLidReading *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
*/

bool QLidFilter::filter(QSensorReading *reading)
{
    return filter(static_cast<QLidReading*>(reading));
}

char const * const QLidSensor::sensorType("QLidSensor");

/*!
    \class QLidSensor
    \ingroup sensors_type
    \inmodule QtSensors
    \since 5.9
    \internal

    \brief The QLidSensor class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a QLidReading instead
    of a QSensorReading.

    For details about how the sensor works, see \l QLidReading.

    \sa QLidReading
*/

/*!
    Construct the sensor as a child of \a parent.
*/
QLidSensor::QLidSensor(QObject *parent)
    : QSensor(QLidSensor::sensorType, parent)
{
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
*/
QLidSensor::~QLidSensor()
{
}

/*!
    \fn QLidSensor::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
*/

QLidReading *QLidSensor::reading() const
{
    return static_cast<QLidReading*>(QSensor::reading());
}

QT_END_NAMESPACE

#include "moc_qlidsensor.cpp"
