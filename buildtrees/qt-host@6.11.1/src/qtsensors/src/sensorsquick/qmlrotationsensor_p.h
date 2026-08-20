// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLROTATIONSENSOR_P_H
#define QMLROTATIONSENSOR_P_H

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

class QRotationSensor;

class Q_SENSORSQUICK_EXPORT QmlRotationSensor : public QmlSensor
{
    Q_OBJECT
    Q_PROPERTY(bool hasZ READ hasZ NOTIFY hasZChanged)
    QML_NAMED_ELEMENT(RotationSensor)
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlRotationSensor(QObject *parent = 0);
    ~QmlRotationSensor();

    bool hasZ() const;
    QSensor *sensor() const override;

Q_SIGNALS:
    void hasZChanged(bool hasZ);

private:
    QRotationSensor *m_sensor;
    QmlSensorReading *createReading() const override;
};

class Q_SENSORSQUICK_EXPORT QmlRotationSensorReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x NOTIFY xChanged BINDABLE bindableX)
    Q_PROPERTY(qreal y READ y NOTIFY yChanged BINDABLE bindableY)
    Q_PROPERTY(qreal z READ z NOTIFY zChanged BINDABLE bindableZ)
    QML_NAMED_ELEMENT(RotationReading)
    QML_UNCREATABLE("Cannot create RotationReading")
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlRotationSensorReading(QRotationSensor *sensor);
    ~QmlRotationSensorReading();

    qreal x() const;
    QBindable<qreal> bindableX() const;
    qreal y() const;
    QBindable<qreal> bindableY() const;
    qreal z() const;
    QBindable<qreal> bindableZ() const;

Q_SIGNALS:
    void xChanged();
    void yChanged();
    void zChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;
    QRotationSensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlRotationSensorReading, qreal,
                               m_x, &QmlRotationSensorReading::xChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlRotationSensorReading, qreal,
                               m_y, &QmlRotationSensorReading::yChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlRotationSensorReading, qreal,
                               m_z, &QmlRotationSensorReading::zChanged)
};

QT_END_NAMESPACE
#endif
