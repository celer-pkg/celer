// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QSENSORBACKEND_H
#define QSENSORBACKEND_H

#include <QtSensors/qsensor.h>
#include <QtSensors/qsensormanager.h>

QT_BEGIN_NAMESPACE

class QSensorBackendPrivate;

class Q_SENSORS_EXPORT QSensorBackend : public QObject
{
    Q_OBJECT
public:
    explicit QSensorBackend(QSensor *sensor, QObject *parent = nullptr);
    virtual ~QSensorBackend();

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool isFeatureSupported(QSensor::Feature feature) const;

    // used by the backend to set metadata properties
    void addDataRate(qreal min, qreal max);
    void setDataRates(const QSensor *otherSensor);
    void addOutputRange(qreal min, qreal max, qreal accuracy);
    void setDescription(const QString &description);

    template <typename T>
    T *setReading(T *readingClass)
    {
        if (!readingClass)
            readingClass = new T(this);
        setReadings(readingClass, new T(this), new T(this));
        return readingClass;
    }

    QSensorReading *reading() const;
    QSensor *sensor() const;

    // used by the backend to inform us of events
    void newReadingAvailable();
    void sensorStopped();
    void sensorBusy(bool busy = true);
    void sensorError(int error);

private:
    void setReadings(QSensorReading *device, QSensorReading *filter, QSensorReading *cache);

    Q_DECLARE_PRIVATE(QSensorBackend)
    Q_DISABLE_COPY(QSensorBackend)
};

QT_END_NAMESPACE

#endif

