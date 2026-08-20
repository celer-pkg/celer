// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGYROSCOPE_H
#define QGYROSCOPE_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QGyroscopeReadingPrivate;

class Q_SENSORS_EXPORT QGyroscopeReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x)
    Q_PROPERTY(qreal y READ y)
    Q_PROPERTY(qreal z READ z)
    DECLARE_READING(QGyroscopeReading)
public:
    qreal x() const;
    void setX(qreal x);

    qreal y() const;
    void setY(qreal y);

    qreal z() const;
    void setZ(qreal z);
};

class Q_SENSORS_EXPORT QGyroscopeFilter : public QSensorFilter
{
public:
    virtual bool filter(QGyroscopeReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QGyroscope : public QSensor
{
    Q_OBJECT
public:
    explicit QGyroscope(QObject *parent = nullptr);
    virtual ~QGyroscope();
    QGyroscopeReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QGyroscope)
};

QT_END_NAMESPACE

#endif

