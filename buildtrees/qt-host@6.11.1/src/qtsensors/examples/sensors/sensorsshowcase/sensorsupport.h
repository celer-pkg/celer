// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef SENSORSUPPORT_H
#define SENSORSUPPORT_H

#include <QObject>
#include <QtQmlIntegration>
#include <QtSensors/QtSensors>

class SensorSupport : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    QML_UNCREATABLE("SensorSupport is a utility class")
public:
    explicit SensorSupport(QObject *parent = nullptr) : QObject(parent) { }

    Q_INVOKABLE static bool hasAccelerometer()
    {
        return !QSensor::sensorsForType(QAccelerometer::sensorType).empty();
    }
    Q_INVOKABLE static bool hasCompass()
    {
        return !QSensor::sensorsForType(QCompass::sensorType).empty();
    }
    Q_INVOKABLE static bool hasGyroscope()
    {
        return !QSensor::sensorsForType(QGyroscope::sensorType).empty();
    }
    Q_INVOKABLE static bool hasMagnetometer()
    {
        return !QSensor::sensorsForType(QMagnetometer::sensorType).empty();
    }
    Q_INVOKABLE static bool hasProximity()
    {
        return !QSensor::sensorsForType(QProximitySensor::sensorType).empty();
    }
};

#endif // SENSORSUPPORT_H
