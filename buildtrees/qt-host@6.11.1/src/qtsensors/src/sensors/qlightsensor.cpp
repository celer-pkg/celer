// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlightsensor.h"
#include "qlightsensor_p.h"

QT_BEGIN_NAMESPACE

IMPLEMENT_READING(QLightReading)

/*!
    \class QLightReading
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 5.1

    \brief The QLightReading class represents one reading from the
           light sensor.

    \section2 QLightReading Units
    The light sensor returns the intensity of the light in lux.
*/

/*!
    \property QLightReading::lux
    \brief the light level.

    Measured as lux.
    \sa {QLightReading Units}
*/

qreal QLightReading::lux() const
{
    return d->lux;
}

/*!
    Sets the light level to \a lux.
*/
void QLightReading::setLux(qreal lux)
{
    d->lux = lux;
}

// =====================================================================

/*!
    \class QLightFilter
    \ingroup sensors_filter
    \inmodule QtSensors
    \since 5.1

    \brief The QLightFilter class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to QLightReading
    instead of QSensorReading.
*/

/*!
    \fn QLightFilter::filter(QLightReading *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
*/

bool QLightFilter::filter(QSensorReading *reading)
{
    return filter(static_cast<QLightReading*>(reading));
}

char const * const QLightSensor::sensorType("QLightSensor");

/*!
    \class QLightSensor
    \ingroup sensors_type
    \inmodule QtSensors
    \since 5.1

    \brief The QLightSensor class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a QLightReading instead of a QSensorReading.

    For details about how the sensor works, see \l QLightReading.

    \sa QLightReading
*/

/*!
    Construct the sensor as a child of \a parent.
*/
QLightSensor::QLightSensor(QObject *parent)
    : QSensor(QLightSensor::sensorType, *new QLightSensorPrivate, parent)
{
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
*/
QLightSensor::~QLightSensor()
{
}

/*!
    \fn QLightSensor::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
*/

QLightReading *QLightSensor::reading() const
{
    return static_cast<QLightReading*>(QSensor::reading());
}

/*!
    \property QLightSensor::fieldOfView
    \brief a value indicating the field of view.

    This is an angle that represents the field of view of the sensor.

    Not all light sensor support retrieving their field of view. For sensors
    that don't support this property, the value will be 0. Whether the field of
    view is supported can be checked with QSensor::isFeatureSupported() and the
    QSensor::FieldOfView flag.
*/

qreal QLightSensor::fieldOfView() const
{
    Q_D(const QLightSensor);
    return d->fieldOfView;
}

/*!
    \since 5.1

    Sets the field of view to \a fieldOfView. This is to be called from the
    backend.
*/
void QLightSensor::setFieldOfView(qreal fieldOfView)
{
    Q_D(QLightSensor);
    if (d->fieldOfView != fieldOfView) {
        d->fieldOfView = fieldOfView;
        emit fieldOfViewChanged(fieldOfView);
    }
}

QT_END_NAMESPACE

#include "moc_qlightsensor.cpp"
