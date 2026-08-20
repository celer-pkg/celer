// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTILTSENSOR_H
#define QTILTSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QTiltReadingPrivate;

class Q_SENSORS_EXPORT QTiltReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal yRotation READ yRotation)
    Q_PROPERTY(qreal xRotation READ xRotation)
    DECLARE_READING(QTiltReading)

public:
    qreal yRotation() const;
    void setYRotation(qreal y);

    qreal xRotation() const;
    void setXRotation(qreal x);

};

class Q_SENSORS_EXPORT QTiltFilter : public QSensorFilter
{
public:
    virtual bool filter(QTiltReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QTiltSensor : public QSensor
{
    Q_OBJECT
public:
    explicit QTiltSensor(QObject *parent = nullptr);
    ~QTiltSensor();
    QTiltReading *reading() const;
    static char const * const sensorType;

    Q_INVOKABLE void calibrate();

private:
    Q_DISABLE_COPY(QTiltSensor)
};

QT_END_NAMESPACE
#endif
