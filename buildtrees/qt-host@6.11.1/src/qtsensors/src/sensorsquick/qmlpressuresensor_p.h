// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QMLPRESSURESENSOR_P_H
#define QMLPRESSURESENSOR_P_H

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

class QPressureSensor;

class Q_SENSORSQUICK_EXPORT QmlPressureSensor : public QmlSensor
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PressureSensor)
    QML_ADDED_IN_VERSION(5,1)
public:
    explicit QmlPressureSensor(QObject *parent = 0);
    ~QmlPressureSensor();

    QSensor *sensor() const override;

private:
    QmlSensorReading *createReading() const override;

    QPressureSensor *m_sensor;
};

class Q_SENSORSQUICK_EXPORT QmlPressureReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal pressure READ pressure NOTIFY pressureChanged BINDABLE bindablePressure)
    Q_PROPERTY(qreal temperature READ temperature
               NOTIFY temperatureChanged REVISION 1 BINDABLE bindableTemperature)
    QML_NAMED_ELEMENT(PressureReading)
    QML_UNCREATABLE("Cannot create PressureReading")
    QML_ADDED_IN_VERSION(5,1)
public:
    explicit QmlPressureReading(QPressureSensor *sensor);
    ~QmlPressureReading();

    qreal pressure() const;
    QBindable<qreal> bindablePressure() const;
    qreal temperature() const;
    QBindable<qreal> bindableTemperature() const;

Q_SIGNALS:
    void pressureChanged();
    Q_REVISION(1) void temperatureChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;

    QPressureSensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlPressureReading, qreal,
                               m_pressure, &QmlPressureReading::pressureChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlPressureReading, qreal,
                               m_temperature, &QmlPressureReading::temperatureChanged)
};

QT_END_NAMESPACE
#endif
