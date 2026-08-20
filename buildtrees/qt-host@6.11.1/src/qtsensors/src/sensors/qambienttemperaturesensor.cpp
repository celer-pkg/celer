// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include <qambienttemperaturesensor.h>
#include "qambienttemperaturesensor_p.h"

QT_BEGIN_NAMESPACE

IMPLEMENT_READING(QAmbientTemperatureReading)

/*!
    \class QAmbientTemperatureReading
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 5.1

    \brief The QAmbientTemperatureReading class holds readings of the ambient temperature.

    The ambient (room) temperature is the temperature in degree Celsius.
*/

/*!
    \property QAmbientTemperatureReading::temperature
    \brief The ambient temperature

    Measured in degree Celsius.
*/

qreal QAmbientTemperatureReading::temperature() const
{
    return d->temperature;
}

/*!
    Sets ambient temperature to \a temperature.
*/
void QAmbientTemperatureReading::setTemperature(qreal temperature)
{
    d->temperature = temperature;
}

// =====================================================================

/*!
    \class QAmbientTemperatureFilter
    \ingroup sensors_filter
    \inmodule QtSensors
    \since 5.1

    \brief The QAmbientTemperatureFilter class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to QAmbientTemperatureReading
    instead of QSensorReading.
*/

/*!
    \fn QAmbientTemperatureFilter::filter(QAmbientTemperatureReading *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
*/

bool QAmbientTemperatureFilter::filter(QSensorReading *reading)
{
    return filter(static_cast<QAmbientTemperatureReading*>(reading));
}

char const * const QAmbientTemperatureSensor::sensorType("QAmbientTemperatureSensor");

/*!
    \class QAmbientTemperatureSensor
    \ingroup sensors_type
    \inmodule QtSensors
    \since 5.1

    \brief The QAmbientTemperatureSensor class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a QAmbientTemperatureReading instead of a QSensorReading.

    For details about how the sensor works, see \l QAmbientTemperatureReading.

    \sa QAmbientTemperatureReading
*/

/*!
    Construct the sensor as a child of \a parent.
*/
QAmbientTemperatureSensor::QAmbientTemperatureSensor(QObject *parent)
    : QSensor(QAmbientTemperatureSensor::sensorType, parent)
{
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
*/
QAmbientTemperatureSensor::~QAmbientTemperatureSensor()
{
}

/*!
    \fn QAmbientTemperatureSensor::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
*/

QAmbientTemperatureReading *QAmbientTemperatureSensor::reading() const
{
    return static_cast<QAmbientTemperatureReading*>(QSensor::reading());
}

QT_END_NAMESPACE

#include "moc_qambienttemperaturesensor.cpp"
