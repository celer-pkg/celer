// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROXIMITYSENSOR_H
#define QPROXIMITYSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QProximityReadingPrivate;

class Q_SENSORS_EXPORT QProximityReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(bool close READ close)
    DECLARE_READING(QProximityReading)
public:
    bool close() const;
    void setClose(bool close);
};

class Q_SENSORS_EXPORT QProximityFilter : public QSensorFilter
{
public:
    virtual bool filter(QProximityReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QProximitySensor : public QSensor
{
    Q_OBJECT
public:
    explicit QProximitySensor(QObject *parent = nullptr);
    virtual ~QProximitySensor();
    QProximityReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QProximitySensor)
};

QT_END_NAMESPACE

#endif

