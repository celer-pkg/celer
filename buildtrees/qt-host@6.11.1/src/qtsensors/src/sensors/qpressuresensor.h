// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QPRESSURESENSOR_H
#define QPRESSURESENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QPressureReadingPrivate;

class Q_SENSORS_EXPORT QPressureReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal pressure READ pressure)
    Q_PROPERTY(qreal temperature READ temperature)
    DECLARE_READING(QPressureReading)
public:
    qreal pressure() const;
    void setPressure(qreal pressure);

    qreal temperature() const;
    void setTemperature(qreal temperature);
};

class Q_SENSORS_EXPORT QPressureFilter : public QSensorFilter
{
public:
    virtual bool filter(QPressureReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QPressureSensor : public QSensor
{
    Q_OBJECT
public:
    explicit QPressureSensor(QObject *parent = nullptr);
    ~QPressureSensor();
    QPressureReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QPressureSensor)
};

QT_END_NAMESPACE

#endif
