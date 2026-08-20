// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtSensors/qsensor.h>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>

void start()
{
//! [Starting a sensor]
// start the sensor
QSensor sensor("QAccelerometer");
sensor.start();

// later
QSensorReading *reading = sensor.reading();
qreal x = reading->property("x").value<qreal>();
qreal y = reading->value(1).value<qreal>();
//! [Starting a sensor]

    Q_UNUSED(x);
    Q_UNUSED(y);
}

class MyObject : public QObject
{
    void findSensors()
    {
        //! [Find sensors]
        QList<QSensor*> mySensorList;
        for (const QByteArray &type : QSensor::sensorTypes()) {
            qDebug() << "Found a sensor type:" << type;
            for (const QByteArray &identifier : QSensor::sensorsForType(type)) {
                qDebug() << "    " << "Found a sensor of that type:" << identifier;
                QSensor* sensor = new QSensor(type, this);
                sensor->setIdentifier(identifier);
                mySensorList.append(sensor);
            }
        }
        //! [Find sensors]
        //! [Print reading properties]
        for (QSensor* sensor : mySensorList) {
            const int firstProperty = QSensorReading::staticMetaObject.propertyOffset();
            // Connect to backend first in case start() hasn't been called yet
            if (!sensor->connectToBackend())
                continue;
            qDebug() << "Sensor" << sensor->identifier() << "reading properties:";
            QSensorReading *reading = sensor->reading();
            if (reading) {
                const QMetaObject *mo = reading->metaObject();
                for (int i = firstProperty; i < mo->propertyCount(); ++i) {
                    QByteArray name = mo->property(i).name();
                    qDebug() << "    " << name << reading->property(name).toByteArray();
                }
            }
        }
        //! [Print reading properties]
    }
};
