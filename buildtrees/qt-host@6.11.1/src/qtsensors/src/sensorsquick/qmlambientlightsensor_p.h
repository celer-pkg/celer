// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLAMBIENTLIGHTSENSOR_P_H
#define QMLAMBIENTLIGHTSENSOR_P_H

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
#include <QtSensors/QAmbientLightSensor>

QT_BEGIN_NAMESPACE

class QAmbientLightSensor;

class Q_SENSORSQUICK_EXPORT QmlAmbientLightSensor : public QmlSensor
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AmbientLightSensor)
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlAmbientLightSensor(QObject *parent = 0);
    ~QmlAmbientLightSensor();

    QSensor *sensor() const override;

private:
    QAmbientLightSensor *m_sensor;
    QmlSensorReading *createReading() const override;

};

class Q_SENSORSQUICK_EXPORT QmlAmbientLightSensorReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(QAmbientLightReading::LightLevel lightLevel READ lightLevel
               NOTIFY lightLevelChanged BINDABLE bindableLightLevel)
    QML_NAMED_ELEMENT(AmbientLightReading)
    QML_UNCREATABLE("Cannot create AmbientLightReading")
    QML_ADDED_IN_VERSION(5,0)
public:

    explicit QmlAmbientLightSensorReading(QAmbientLightSensor *sensor);
    ~QmlAmbientLightSensorReading();

    QAmbientLightReading::LightLevel lightLevel() const;
    QBindable<QAmbientLightReading::LightLevel> bindableLightLevel() const;

Q_SIGNALS:
    void lightLevelChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;
    QAmbientLightSensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlAmbientLightSensorReading, QAmbientLightReading::LightLevel,
                               m_lightLevel, &QmlAmbientLightSensorReading::lightLevelChanged)
};

QT_END_NAMESPACE
#endif
