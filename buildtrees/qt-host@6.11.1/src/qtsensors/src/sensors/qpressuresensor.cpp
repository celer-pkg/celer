// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include <qpressuresensor.h>
#include "qpressuresensor_p.h"

QT_BEGIN_NAMESPACE

IMPLEMENT_READING(QPressureReading)

/*!
    \class QPressureReading
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 5.1

    \brief The QPressureReading class holds readings from the pressure sensor.

    \section2 QPressureReading Units

    The pressure sensor returns atmospheric pressure values in Pascals.
*/

/*!
    \property QPressureReading::pressure
    \brief The measured atmospheric pressure.

    Returned as Pascals.
    \sa {QPressureReading Units}
*/

qreal QPressureReading::pressure() const
{
    return d->pressure;
}

/*!
    Sets the pressure to \a pressure.
*/
void QPressureReading::setPressure(qreal pressure)
{
    d->pressure = pressure;
}

/*!
    \property QPressureReading::temperature
    \brief The pressure sensor's temperature.
    \since 5.2

    The temperature is returned in degree Celsius.
    This property, if supported, provides the pressure sensor die temperature.
    Note that this temperature may be (and usually is) different than the temperature
    reported from QAmbientTemperatureSensor.
    Use QSensor::isFeatureSupported() with the QSensor::PressureSensorTemperature
    flag to determine its availability.
*/

qreal QPressureReading::temperature() const
{
    return d->temperature;
}

/*!
    Sets the pressure sensor's temperature to \a temperature.
    \since 5.2
*/
void QPressureReading::setTemperature(qreal temperature)
{
    d->temperature =  temperature;
}

// =====================================================================

/*!
    \class QPressureFilter
    \ingroup sensors_filter
    \inmodule QtSensors
    \since 5.1

    \brief The QPressureFilter class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to QPressureReading
    instead of QSensorReading.
*/

/*!
    \fn QPressureFilter::filter(QPressureReading *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
*/

bool QPressureFilter::filter(QSensorReading *reading)
{
    return filter(static_cast<QPressureReading*>(reading));
}

char const * const QPressureSensor::sensorType("QPressureSensor");

/*!
    \class QPressureSensor
    \ingroup sensors_type
    \inmodule QtSensors
    \since 5.1

    \brief The QPressureSensor class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a QPressureReading instead of a QSensorReading.

    For details about how the sensor works, see \l QPressureReading.

    \sa QPressureReading
*/

/*!
    Construct the sensor as a child of \a parent.
*/
QPressureSensor::QPressureSensor(QObject *parent)
    : QSensor(QPressureSensor::sensorType, parent)
{
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
*/
QPressureSensor::~QPressureSensor()
{
}

/*!
    \fn QPressureSensor::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
*/

QPressureReading *QPressureSensor::reading() const
{
    return static_cast<QPressureReading*>(QSensor::reading());
}

QT_END_NAMESPACE

#include "moc_qpressuresensor.cpp"
