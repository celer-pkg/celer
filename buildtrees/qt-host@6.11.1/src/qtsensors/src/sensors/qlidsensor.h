// Copyright (C) 2016 Canonical, Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QLIDSENSOR_H
#define QLIDSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QLidReadingPrivate;

class Q_SENSORS_EXPORT QLidReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(bool backLidClosed READ backLidClosed)
    Q_PROPERTY(bool frontLidClosed READ frontLidClosed)
    DECLARE_READING(QLidReading)
public:

    bool backLidClosed() const;
    void setBackLidClosed(bool closed);

    bool frontLidClosed() const;
    void setFrontLidClosed(bool closed);

Q_SIGNALS:
    void backLidChanged(bool closed);
    void frontLidChanged(bool closed);
};

class Q_SENSORS_EXPORT QLidFilter : public QSensorFilter
{
public:
    virtual bool filter(QLidReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QLidSensor : public QSensor
{
    Q_OBJECT
public:
    explicit QLidSensor(QObject *parent = nullptr);
    ~QLidSensor();
    QLidReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QLidSensor)
};

QT_END_NAMESPACE

#endif
