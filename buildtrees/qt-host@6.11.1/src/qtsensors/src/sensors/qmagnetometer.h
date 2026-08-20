// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMAGNETOMETER_H
#define QMAGNETOMETER_H

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class QMagnetometerReadingPrivate;

class Q_SENSORS_EXPORT QMagnetometerReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x)
    Q_PROPERTY(qreal y READ y)
    Q_PROPERTY(qreal z READ z)
    Q_PROPERTY(qreal calibrationLevel READ calibrationLevel)
    DECLARE_READING(QMagnetometerReading)
public:
    qreal x() const;
    void setX(qreal x);

    qreal y() const;
    void setY(qreal y);

    qreal z() const;
    void setZ(qreal z);

    qreal calibrationLevel() const;
    void setCalibrationLevel(qreal calibrationLevel);
};

class Q_SENSORS_EXPORT QMagnetometerFilter : public QSensorFilter
{
public:
    virtual bool filter(QMagnetometerReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class QMagnetometerPrivate;

class Q_SENSORS_EXPORT QMagnetometer : public QSensor
{
    Q_OBJECT
    Q_PROPERTY(bool returnGeoValues READ returnGeoValues WRITE setReturnGeoValues NOTIFY returnGeoValuesChanged)
public:
    explicit QMagnetometer(QObject *parent = nullptr);
    virtual ~QMagnetometer();
    QMagnetometerReading *reading() const;
    static char const * const sensorType;

    bool returnGeoValues() const;
    void setReturnGeoValues(bool returnGeoValues);

Q_SIGNALS:
    void returnGeoValuesChanged(bool returnGeoValues);

private:
    Q_DECLARE_PRIVATE(QMagnetometer)
    Q_DISABLE_COPY(QMagnetometer)
};

QT_END_NAMESPACE

#endif

