// Copyright (C) 2016 Canonical Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHUMIDITYSENSOR_H
#define QHUMIDITYSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QHumidityReadingPrivate;

class Q_SENSORS_EXPORT QHumidityReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal relativeHumidity READ relativeHumidity)
    Q_PROPERTY(qreal absoluteHumidity READ absoluteHumidity)

    DECLARE_READING(QHumidityReading)
public:
    qreal relativeHumidity() const;
    void setRelativeHumidity(qreal percent);

    qreal absoluteHumidity() const;
    void setAbsoluteHumidity(qreal value);
};

class Q_SENSORS_EXPORT QHumidityFilter : public QSensorFilter
{
public:
    virtual bool filter(QHumidityReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class QHumiditySensorPrivate;

class Q_SENSORS_EXPORT QHumiditySensor : public QSensor
{
    Q_OBJECT
public:
    explicit QHumiditySensor(QObject *parent = nullptr);
    ~QHumiditySensor();

    QHumidityReading *reading() const;
    static char const * const sensorType;

private:
    Q_DECLARE_PRIVATE(QHumiditySensor)
    Q_DISABLE_COPY(QHumiditySensor)
};

QT_END_NAMESPACE

#endif
