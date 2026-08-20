// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLIRPROXIMITYSENSOR_P_H
#define QMLIRPROXIMITYSENSOR_P_H

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

class QIRProximitySensor;

class Q_SENSORSQUICK_EXPORT QmlIRProximitySensor : public QmlSensor
{
    Q_OBJECT
    QML_NAMED_ELEMENT(IRProximitySensor)
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlIRProximitySensor(QObject *parent = 0);
    ~QmlIRProximitySensor();

    QSensor *sensor() const override;

private:
    QIRProximitySensor *m_sensor;
    QmlSensorReading *createReading() const override;
};

class Q_SENSORSQUICK_EXPORT QmlIRProximitySensorReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal reflectance READ reflectance
               NOTIFY reflectanceChanged BINDABLE bindableReflectance)
    QML_NAMED_ELEMENT(IRProximityReading)
    QML_UNCREATABLE("Cannot create IRProximityReading")
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlIRProximitySensorReading(QIRProximitySensor *sensor);
    ~QmlIRProximitySensorReading();

    qreal reflectance() const;
    QBindable<qreal> bindableReflectance() const;

Q_SIGNALS:
    void reflectanceChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;
    QIRProximitySensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlIRProximitySensorReading, qreal,
                               m_reflectance, &QmlIRProximitySensorReading::reflectanceChanged)
};

QT_END_NAMESPACE
#endif
