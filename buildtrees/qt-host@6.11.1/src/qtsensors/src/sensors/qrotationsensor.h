// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QROTATIONSENSOR_H
#define QROTATIONSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QRotationReadingPrivate;

class Q_SENSORS_EXPORT QRotationReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x)
    Q_PROPERTY(qreal y READ y)
    Q_PROPERTY(qreal z READ z)
    DECLARE_READING(QRotationReading)
public:
    qreal x() const;
    qreal y() const;
    qreal z() const;

    void setFromEuler(qreal x, qreal y, qreal z);
};

class Q_SENSORS_EXPORT QRotationFilter : public QSensorFilter
{
public:
    virtual bool filter(QRotationReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class QRotationSensorPrivate;

class Q_SENSORS_EXPORT QRotationSensor : public QSensor
{
    Q_OBJECT
    Q_PROPERTY(bool hasZ READ hasZ NOTIFY hasZChanged)
public:
    explicit QRotationSensor(QObject *parent = nullptr);
    virtual ~QRotationSensor();
    QRotationReading *reading() const;
    static char const * const sensorType;

    bool hasZ() const;
    void setHasZ(bool hasZ);

Q_SIGNALS:
    void hasZChanged(bool hasZ);

private:
    Q_DECLARE_PRIVATE(QRotationSensor)
    Q_DISABLE_COPY(QRotationSensor)
};

QT_END_NAMESPACE

#endif

