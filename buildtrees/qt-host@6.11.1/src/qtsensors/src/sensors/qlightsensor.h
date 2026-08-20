// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLIGHTSENSOR_H
#define QLIGHTSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QLightReadingPrivate;

class Q_SENSORS_EXPORT QLightReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal lux READ lux)
    DECLARE_READING(QLightReading)
public:
    qreal lux() const;
    void setLux(qreal lux);
};

class Q_SENSORS_EXPORT QLightFilter : public QSensorFilter
{
public:
    virtual bool filter(QLightReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class QLightSensorPrivate;

class Q_SENSORS_EXPORT QLightSensor : public QSensor
{
    Q_OBJECT
    Q_PROPERTY(qreal fieldOfView READ fieldOfView NOTIFY fieldOfViewChanged)
public:
    explicit QLightSensor(QObject *parent = nullptr);
    virtual ~QLightSensor();
    QLightReading *reading() const;
    static char const * const sensorType;

    qreal fieldOfView() const;
    void setFieldOfView(qreal fieldOfView);

Q_SIGNALS:
    void fieldOfViewChanged(qreal fieldOfView);

private:
    Q_DECLARE_PRIVATE(QLightSensor)
    Q_DISABLE_COPY(QLightSensor)
};

QT_END_NAMESPACE

#endif

