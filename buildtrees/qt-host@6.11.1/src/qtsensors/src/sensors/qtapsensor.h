// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTAPSENSOR_H
#define QTAPSENSOR_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QTapReadingPrivate;

class Q_SENSORS_EXPORT QTapReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(TapDirection tapDirection READ tapDirection)
    Q_PROPERTY(bool doubleTap READ isDoubleTap)
    DECLARE_READING(QTapReading)
public:
    enum TapDirection {
        Undefined = 0,
        X      = 0x0001,
        Y      = 0x0002,
        Z      = 0x0004,
        X_Pos  = 0x0011,
        Y_Pos  = 0x0022,
        Z_Pos  = 0x0044,
        X_Neg  = 0x0101,
        Y_Neg  = 0x0202,
        Z_Neg  = 0x0404,
        X_Both = 0x0111,
        Y_Both = 0x0222,
        Z_Both = 0x0444
    };
    Q_ENUM(TapDirection)

    TapDirection tapDirection() const;
    void setTapDirection(TapDirection tapDirection);

    bool isDoubleTap() const;
    void setDoubleTap(bool doubleTap);
};

class Q_SENSORS_EXPORT QTapFilter : public QSensorFilter
{
public:
    virtual bool filter(QTapReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class QTapSensorPrivate;

class Q_SENSORS_EXPORT QTapSensor : public QSensor
{
    Q_OBJECT
    Q_PROPERTY(bool returnDoubleTapEvents READ returnDoubleTapEvents WRITE setReturnDoubleTapEvents
               NOTIFY returnDoubleTapEventsChanged)
public:
    explicit QTapSensor(QObject *parent = nullptr);
    virtual ~QTapSensor();
    QTapReading *reading() const;
    static char const * const sensorType;

    bool returnDoubleTapEvents() const;
    void setReturnDoubleTapEvents(bool returnDoubleTapEvents);

Q_SIGNALS:
    void returnDoubleTapEventsChanged(bool returnDoubleTapEvents);

private:
    Q_DECLARE_PRIVATE(QTapSensor)
    Q_DISABLE_COPY(QTapSensor)
};

QT_END_NAMESPACE

#endif
