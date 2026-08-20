// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCOMPASS_H
#define QCOMPASS_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QCompassReadingPrivate;

class Q_SENSORS_EXPORT QCompassReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal azimuth READ azimuth)
    Q_PROPERTY(qreal calibrationLevel READ calibrationLevel)
    DECLARE_READING(QCompassReading)
public:
    qreal azimuth() const;
    void setAzimuth(qreal azimuth);

    qreal calibrationLevel() const;
    void setCalibrationLevel(qreal calibrationLevel);
};

class Q_SENSORS_EXPORT QCompassFilter : public QSensorFilter
{
public:
    virtual bool filter(QCompassReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QCompass : public QSensor
{
    Q_OBJECT
public:
    explicit QCompass(QObject *parent = nullptr);
    virtual ~QCompass();
    QCompassReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QCompass)
};

QT_END_NAMESPACE

#endif

