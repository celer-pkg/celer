// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QMLAMBIENTTEMPERATURESENSOR_P_H
#define QMLAMBIENTTEMPERATURESENSOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmlsensor_p.h"

QT_BEGIN_NAMESPACE

class QAmbientTemperatureSensor;

class Q_SENSORSQUICK_EXPORT QmlAmbientTemperatureSensor : public QmlSensor
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AmbientTemperatureSensor)
    QML_ADDED_IN_VERSION(5,1)
public:
    explicit QmlAmbientTemperatureSensor(QObject *parent = 0);
    ~QmlAmbientTemperatureSensor();

    QSensor *sensor() const override;

private:
    QmlSensorReading *createReading() const override;

    QAmbientTemperatureSensor *m_sensor;
};

class Q_SENSORSQUICK_EXPORT QmlAmbientTemperatureReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal temperature READ temperature
               NOTIFY temperatureChanged BINDABLE bindableTemperature)
    QML_NAMED_ELEMENT(AmbientTemperatureReading)
    QML_UNCREATABLE("Cannot create AmbientTemperatureReading")
    QML_ADDED_IN_VERSION(5,1)
public:
    explicit QmlAmbientTemperatureReading(QAmbientTemperatureSensor *sensor);
    ~QmlAmbientTemperatureReading();

    qreal temperature() const;
    QBindable<qreal> bindableTemperature() const;

Q_SIGNALS:
    void temperatureChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;

    QAmbientTemperatureSensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlAmbientTemperatureReading, qreal,
                               m_temperature, &QmlAmbientTemperatureReading::temperatureChanged)
};

QT_END_NAMESPACE
#endif
