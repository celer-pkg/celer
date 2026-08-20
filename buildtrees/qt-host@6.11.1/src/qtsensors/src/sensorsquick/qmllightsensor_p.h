// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLLIGHTSENSOR_P_H
#define QMLLIGHTSENSOR_P_H

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

class QLightSensor;

class Q_SENSORSQUICK_EXPORT QmlLightSensor : public QmlSensor
{
    Q_OBJECT
    Q_PROPERTY(qreal fieldOfView READ fieldOfView NOTIFY fieldOfViewChanged)
    QML_NAMED_ELEMENT(LightSensor)
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlLightSensor(QObject *parent = 0);
    ~QmlLightSensor();

    qreal fieldOfView() const;
    QSensor *sensor() const override;

Q_SIGNALS:
    void fieldOfViewChanged(qreal fieldOfView);

private:
    QLightSensor *m_sensor;
    QmlSensorReading *createReading() const override;
};

class Q_SENSORSQUICK_EXPORT QmlLightSensorReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal illuminance READ illuminance
               NOTIFY illuminanceChanged BINDABLE bindableIlluminance)
    QML_NAMED_ELEMENT(LightReading)
    QML_UNCREATABLE("Cannot create LightReading")
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlLightSensorReading(QLightSensor *sensor);
    ~QmlLightSensorReading();

    qreal illuminance() const;
    QBindable<qreal> bindableIlluminance() const;

Q_SIGNALS:
    void illuminanceChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;
    QLightSensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlLightSensorReading, qreal,
                               m_illuminance, &QmlLightSensorReading::illuminanceChanged)
};

QT_END_NAMESPACE
#endif
