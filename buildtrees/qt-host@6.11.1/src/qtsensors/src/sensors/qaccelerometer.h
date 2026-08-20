// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QACCELEROMETER_H
#define QACCELEROMETER_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QAccelerometerReadingPrivate;

class Q_SENSORS_EXPORT QAccelerometerReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x)
    Q_PROPERTY(qreal y READ y)
    Q_PROPERTY(qreal z READ z)
    DECLARE_READING(QAccelerometerReading)
public:
    qreal x() const;
    void setX(qreal x);

    qreal y() const;
    void setY(qreal y);

    qreal z() const;
    void setZ(qreal z);
};

class Q_SENSORS_EXPORT QAccelerometerFilter : public QSensorFilter
{
public:
    virtual bool filter(QAccelerometerReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class QAccelerometerPrivate;

class Q_SENSORS_EXPORT QAccelerometer : public QSensor
{
    Q_OBJECT
    Q_PROPERTY(AccelerationMode accelerationMode READ accelerationMode WRITE setAccelerationMode
               NOTIFY accelerationModeChanged)
public:
    explicit QAccelerometer(QObject *parent = nullptr);
    virtual ~QAccelerometer();

    // Keep this enum in sync with QmlAccelerometer::AccelerationMode
    enum AccelerationMode {
        Combined,
        Gravity,
        User
    };
    Q_ENUM(AccelerationMode)

    AccelerationMode accelerationMode() const;
    void setAccelerationMode(AccelerationMode accelerationMode);

    QAccelerometerReading *reading() const;
    static char const * const sensorType;

Q_SIGNALS:
    void accelerationModeChanged(AccelerationMode accelerationMode);

private:
    Q_DECLARE_PRIVATE(QAccelerometer)
    Q_DISABLE_COPY(QAccelerometer)
};

QT_END_NAMESPACE

#endif
