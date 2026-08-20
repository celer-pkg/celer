// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlmagnetometer_p.h"
#include <QtSensors/QMagnetometer>

/*!
    \qmltype Magnetometer
//!    \nativetype QmlMagnetometer
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits Sensor
    \brief The Magnetometer element reports on magnetic field strength
           along the Z, Y and Z axes.

    The Magnetometer element reports on magnetic field strength
    along the Z, Y and Z axes.

    This element wraps the QMagnetometer class. Please see the documentation for
    QMagnetometer for details.

    \sa MagnetometerReading
*/

QmlMagnetometer::QmlMagnetometer(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new QMagnetometer(this))
{
    connect(m_sensor, SIGNAL(returnGeoValuesChanged(bool)),
            this, SIGNAL(returnGeoValuesChanged(bool)));
}

QmlMagnetometer::~QmlMagnetometer()
{
}

QmlSensorReading *QmlMagnetometer::createReading() const
{
    return new QmlMagnetometerReading(m_sensor);
}

QSensor *QmlMagnetometer::sensor() const
{
    return m_sensor;
}

/*!
    \qmlproperty bool Magnetometer::returnGeoValues
    This property holds a value indicating if geomagnetic values should be returned.

    Please see QMagnetometer::returnGeoValues for information about this property.
*/

bool QmlMagnetometer::returnGeoValues() const
{
    return m_sensor->returnGeoValues();
}

void QmlMagnetometer::setReturnGeoValues(bool geo)
{
    m_sensor->setReturnGeoValues(geo);
}

/*!
    \qmltype MagnetometerReading
//!    \nativetype QmlMagnetometerReading
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \inherits SensorReading
    \brief The MagnetometerReading element holds the most recent Magnetometer reading.

    The MagnetometerReading element holds the most recent Magnetometer reading.

    This element wraps the QMagnetometerReading class. Please see the documentation for
    QMagnetometerReading for details.

    This element cannot be directly created.
*/

QmlMagnetometerReading::QmlMagnetometerReading(QMagnetometer *sensor)
    : m_sensor(sensor)
{
}

QmlMagnetometerReading::~QmlMagnetometerReading()
{
}

/*!
    \qmlproperty real MagnetometerReading::x
    This property holds the raw magnetic flux density on the X axis.

    Please see QMagnetometerReading::x for information about this property.
*/

qreal QmlMagnetometerReading::x() const
{
    return m_x;
}

QBindable<qreal> QmlMagnetometerReading::bindableX() const
{
    return &m_x;
}

/*!
    \qmlproperty real MagnetometerReading::y
    This property holds the raw magnetic flux density on the Y axis.

    Please see QMagnetometerReading::y for information about this property.
*/

qreal QmlMagnetometerReading::y() const
{
    return m_y;
}

QBindable<qreal> QmlMagnetometerReading::bindableY() const
{
    return &m_y;
}

/*!
    \qmlproperty real MagnetometerReading::z
    This property holds the raw magnetic flux density on the Z axis.

    Please see QMagnetometerReading::z for information about this property.
*/

qreal QmlMagnetometerReading::z() const
{
    return m_z;
}

QBindable<qreal> QmlMagnetometerReading::bindableZ() const
{
    return &m_z;
}

/*!
    \qmlproperty real MagnetometerReading::calibrationLevel
    This property holds the accuracy of the reading.

    Please see QMagnetometerReading::calibrationLevel for information about this property.
*/

qreal QmlMagnetometerReading::calibrationLevel() const
{
    return m_calibrationLevel;
}

QBindable<qreal> QmlMagnetometerReading::bindableCalibrationLevel() const
{
    return &m_calibrationLevel;
}

QSensorReading *QmlMagnetometerReading::reading() const
{
    return m_sensor->reading();
}

void QmlMagnetometerReading::readingUpdate()
{
    m_x = m_sensor->reading()->x();
    m_y = m_sensor->reading()->y();
    m_z = m_sensor->reading()->z();
    m_calibrationLevel= m_sensor->reading()->calibrationLevel();
}
