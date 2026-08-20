// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLPROXIMITYSENSOR_P_H
#define QMLPROXIMITYSENSOR_P_H

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
#ifdef near
#undef near
#endif
QT_BEGIN_NAMESPACE

class QProximitySensor;

class Q_SENSORSQUICK_EXPORT QmlProximitySensor : public QmlSensor
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ProximitySensor)
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlProximitySensor(QObject *parent = 0);
    ~QmlProximitySensor();

    QSensor *sensor() const override;

private:
    QProximitySensor *m_sensor;
    QmlSensorReading *createReading() const override;
};

class Q_SENSORSQUICK_EXPORT QmlProximitySensorReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(bool near READ near NOTIFY nearChanged BINDABLE bindableNear)
    QML_NAMED_ELEMENT(ProximityReading)
    QML_UNCREATABLE("Cannot create ProximityReading")
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlProximitySensorReading(QProximitySensor *sensor);
    ~QmlProximitySensorReading();

    bool near() const;
    QBindable<bool> bindableNear() const;

Q_SIGNALS:
    void nearChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;
    QProximitySensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlProximitySensorReading, bool,
                               m_near, &QmlProximitySensorReading::nearChanged)
};

QT_END_NAMESPACE
#endif
