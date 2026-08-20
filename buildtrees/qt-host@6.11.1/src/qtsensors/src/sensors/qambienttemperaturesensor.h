// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QAMBIENTTEMPERATURESENSOR_H
#define QAMBIENTTEMPERATURESENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QAmbientTemperatureReadingPrivate;

class Q_SENSORS_EXPORT QAmbientTemperatureReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal temperature READ temperature)
    DECLARE_READING(QAmbientTemperatureReading)
public:
    qreal temperature() const;
    void setTemperature(qreal temperature);
};

class Q_SENSORS_EXPORT QAmbientTemperatureFilter : public QSensorFilter
{
public:
    virtual bool filter(QAmbientTemperatureReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QAmbientTemperatureSensor : public QSensor
{
    Q_OBJECT
public:
    explicit QAmbientTemperatureSensor(QObject *parent = nullptr);
    ~QAmbientTemperatureSensor();
    QAmbientTemperatureReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QAmbientTemperatureSensor)
};

QT_END_NAMESPACE

#endif
