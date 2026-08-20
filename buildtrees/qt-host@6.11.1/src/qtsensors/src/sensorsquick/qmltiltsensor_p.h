// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLTILTSENSOR_P_H
#define QMLTILTSENSOR_P_H

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
#include <QtSensors/QTiltSensor>

QT_BEGIN_NAMESPACE

class QTiltSensor;

class Q_SENSORSQUICK_EXPORT QmlTiltSensor : public QmlSensor
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TiltSensor)
    QML_ADDED_IN_VERSION(5,0)
public:

    explicit QmlTiltSensor(QObject *parent = 0);
    ~QmlTiltSensor();
    Q_INVOKABLE void calibrate();

    QSensor *sensor() const override;

private:
    QTiltSensor *m_sensor;
    QmlSensorReading *createReading() const override;
};

class Q_SENSORSQUICK_EXPORT QmlTiltSensorReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal yRotation READ yRotation NOTIFY yRotationChanged BINDABLE bindableYRotation)
    Q_PROPERTY(qreal xRotation READ xRotation NOTIFY xRotationChanged BINDABLE bindableXRotation)
    QML_NAMED_ELEMENT(TiltReading)
    QML_UNCREATABLE("Cannot create TiltReading")
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlTiltSensorReading(QTiltSensor *sensor);
    ~QmlTiltSensorReading();

    qreal yRotation() const;
    QBindable<qreal> bindableYRotation() const;
    qreal xRotation() const;
    QBindable<qreal> bindableXRotation() const;

Q_SIGNALS:
    void yRotationChanged();
    void xRotationChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;
    QTiltSensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlTiltSensorReading, qreal,
                               m_yRotation, &QmlTiltSensorReading::yRotationChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlTiltSensorReading, qreal,
                               m_xRotation, &QmlTiltSensorReading::xRotationChanged)
};

QT_END_NAMESPACE
#endif
