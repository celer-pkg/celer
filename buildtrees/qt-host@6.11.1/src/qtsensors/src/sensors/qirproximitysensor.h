// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QIRPROXIMITYSENSOR_H
#define QIRPROXIMITYSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QIRProximityReadingPrivate;

class Q_SENSORS_EXPORT QIRProximityReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal reflectance READ reflectance)
    DECLARE_READING(QIRProximityReading)
public:
    qreal reflectance() const;
    void setReflectance(qreal reflectance);
};

class Q_SENSORS_EXPORT QIRProximityFilter : public QSensorFilter
{
public:
    virtual bool filter(QIRProximityReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QIRProximitySensor : public QSensor
{
    Q_OBJECT
public:
    explicit QIRProximitySensor(QObject *parent = nullptr);
    virtual ~QIRProximitySensor();
    QIRProximityReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QIRProximitySensor)
};

QT_END_NAMESPACE

#endif

