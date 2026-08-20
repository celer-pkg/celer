// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qcompass.h"
#include "qcompass_p.h"

QT_BEGIN_NAMESPACE

IMPLEMENT_READING(QCompassReading)

/*!
    \class QCompassReading
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 5.1

    \brief The QCompassReading class represents one reading from a
           compass.

    \section2 QCompassReading Units
    The compass returns the azimuth of the device as degrees from
    magnetic north in a clockwise direction based on the top of the device,
    as defined by QScreen::nativeOrientation.
    There is also a value to indicate the calibration status of the device.
    If the device is not calibrated the azimuth may not be accurate.

    Digital compasses are susceptible to magnetic interference and may need
    calibration after being placed near anything that emits a magnetic force.
    Accuracy of the compass can be affected by any ferrous materials that are nearby.

    The calibration status of the device is measured as a number from 0 to 1.
    A value of 1 is the highest level that the device can support and 0 is
    the worst.
*/

/*!
    \property QCompassReading::azimuth
    \brief the azimuth of the device.

    Measured in degrees from magnetic north in a clockwise direction based on
    the top of the device, as defined by QScreen::nativeOrientation.
    \sa {QCompassReading Units}
*/

qreal QCompassReading::azimuth() const
{
    return d->azimuth;
}

/*!
    Sets the \a azimuth of the device.

    \sa {QCompassReading Units}
*/
void QCompassReading::setAzimuth(qreal azimuth)
{
    d->azimuth = azimuth;
}

/*!
    \property QCompassReading::calibrationLevel
    \brief the calibration level of the reading.

    Measured as a value from 0 to 1 with higher values being better.
    \sa {QCompassReading Units}
*/

qreal QCompassReading::calibrationLevel() const
{
    return d->calibrationLevel;
}

/*!
    Sets the calibration level of the reading to \a calibrationLevel.
*/
void QCompassReading::setCalibrationLevel(qreal calibrationLevel)
{
    d->calibrationLevel = calibrationLevel;
}

// =====================================================================

/*!
    \class QCompassFilter
    \ingroup sensors_filter
    \inmodule QtSensors
    \since 5.1

    \brief The QCompassFilter class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to QCompassReading
    instead of QSensorReading.
*/

/*!
    \fn QCompassFilter::filter(QCompassReading *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
*/

bool QCompassFilter::filter(QSensorReading *reading)
{
    return filter(static_cast<QCompassReading*>(reading));
}

char const * const QCompass::sensorType("QCompass");

/*!
    \class QCompass
    \ingroup sensors_type
    \inmodule QtSensors
    \since 5.1

    \brief The QCompass class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a QCompassReading instead of a QSensorReading.

    For details about how the sensor works, see \l QCompassReading.

    \sa QCompassReading
*/

/*!
    Construct the sensor as a child of \a parent.
*/
QCompass::QCompass(QObject *parent)
    : QSensor(QCompass::sensorType, parent)
{
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
*/
QCompass::~QCompass()
{
}

/*!
    \fn QCompass::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
*/

QCompassReading *QCompass::reading() const
{
    return static_cast<QCompassReading*>(QSensor::reading());
}

QT_END_NAMESPACE

#include "moc_qcompass.cpp"
