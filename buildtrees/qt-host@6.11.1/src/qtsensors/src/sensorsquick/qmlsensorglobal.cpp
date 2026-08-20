// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlsensorglobal_p.h"
#include <QtSensors/QSensor>

QT_BEGIN_NAMESPACE

/*!
    \qmltype QmlSensors
//!    \nativetype QmlSensorGlobal
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \brief The QmlSensors singleton provides the module API.

    The QmlSensors singleton provides the module API.

    This element cannot be directly created, but its functionality
    can be accessed as a QML singleton as illustrated below:

    \code
    import QtSensors
    import QtSensors as Sensors
    ...
        Component.onCompleted: {
            var types = Sensors.QmlSensors.sensorTypes();
            console.log(types.join(", "));
        }
    \endcode
*/

QmlSensorGlobal::QmlSensorGlobal(QObject *parent)
    : QObject(parent)
    , m_sensor(new QSensor(QByteArray(), this))
{
    connect(m_sensor, SIGNAL(availableSensorsChanged()), this, SIGNAL(availableSensorsChanged()));
}

QmlSensorGlobal::~QmlSensorGlobal()
{
}

/*!
    \qmlmethod list<string> QmlSensors::sensorTypes()
    Returns a list of the sensor types that have been registered.

    Please see QSensor::sensorTypes() for information.
*/
QStringList QmlSensorGlobal::sensorTypes() const
{
    QStringList ret;
    const QList<QByteArray> sensorTypes = QSensor::sensorTypes();
    ret.reserve(sensorTypes.size());
    for (const QByteArray &type : sensorTypes)
        ret << QString::fromLocal8Bit(type);
    return ret;
}

/*!
    \qmlmethod list<string> QmlSensors::sensorsForType(type)
    Returns a list of the sensor identifiers that have been registered for \a type.

    Please see QSensor::sensorsForType() for information.
*/
QStringList QmlSensorGlobal::sensorsForType(const QString &type) const
{
    QStringList ret;
    const QList<QByteArray> sensors = QSensor::sensorsForType(type.toLocal8Bit());
    ret.reserve(sensors.size());
    for (const QByteArray &identifier : sensors)
        ret << QString::fromLocal8Bit(identifier);
    return ret;
}

/*!
    \qmlmethod string QmlSensors::defaultSensorForType(type)
    Returns the default sensor identifier that has been registered for \a type.

    Please see QSensor::defaultSensorForType() for information.
*/
QString QmlSensorGlobal::defaultSensorForType(const QString &type) const
{
    return QString::fromLocal8Bit(QSensor::defaultSensorForType(type.toLocal8Bit()));
}

QT_END_NAMESPACE
