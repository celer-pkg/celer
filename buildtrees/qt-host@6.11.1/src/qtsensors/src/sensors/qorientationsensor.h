// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QORIENTATIONSENSOR_H
#define QORIENTATIONSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QOrientationReadingPrivate;

class Q_SENSORS_EXPORT QOrientationReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(Orientation orientation READ orientation)
    DECLARE_READING(QOrientationReading)
public:
    enum Orientation {
        Undefined = 0,
        TopUp,
        TopDown,
        LeftUp,
        RightUp,
        FaceUp,
        FaceDown
    };
    Q_ENUM(Orientation)

    Orientation orientation() const;
    void setOrientation(Orientation orientation);
};

class Q_SENSORS_EXPORT QOrientationFilter : public QSensorFilter
{
public:
    virtual bool filter(QOrientationReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT QOrientationSensor : public QSensor
{
    Q_OBJECT
public:
    explicit QOrientationSensor(QObject *parent = nullptr);
    virtual ~QOrientationSensor();
    QOrientationReading *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY(QOrientationSensor)
};

QT_END_NAMESPACE

#endif
