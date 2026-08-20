// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QAMBIENTLIGHTSENSOR_H
#define QAMBIENTLIGHTSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QAmbientLightReadingPrivate;

class Q_SENSORS_EXPORT QAmbientLightReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(LightLevel lightLevel READ lightLevel)
    DECLARE_READING(QAmbientLightReading)
public:
    enum LightLevel {
        Undefined = 0,
        Dark,
        Twilight,
        Light,
        Bright,
        Sunny
    };
    Q_ENUM(LightLevel)

    LightLevel lightLevel() const;
    void setLightLevel(LightLevel lightLevel);
};

class Q_SENSORS_EXPORT QAmbientLightFilter : public QSensorFilter
{
public:
    virtual bool filter(QAmbientLightReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QAmbientLightSensor : public QSensor
{
    Q_OBJECT
public:
    explicit QAmbientLightSensor(QObject *parent = nullptr);
    virtual ~QAmbientLightSensor();
    QAmbientLightReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QAmbientLightSensor)
};

QT_END_NAMESPACE

#endif
