// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLACCELEROMETER_P_H
#define QMLACCELEROMETER_P_H

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

class QAccelerometer;

class Q_SENSORSQUICK_EXPORT QmlAccelerometer : public QmlSensor
{
    Q_OBJECT
    Q_PROPERTY(AccelerationMode accelerationMode READ accelerationMode WRITE setAccelerationMode
               NOTIFY accelerationModeChanged REVISION 1)
    QML_NAMED_ELEMENT(Accelerometer)
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlAccelerometer(QObject *parent = 0);
    ~QmlAccelerometer();

    // Keep this enum in sync with QAccelerometer::AccelerationMode
    enum AccelerationMode {
        Combined,
        Gravity,
        User
    };
    Q_ENUM(AccelerationMode)

    AccelerationMode accelerationMode() const;
    void setAccelerationMode(AccelerationMode accelerationMode);

    QSensor *sensor() const override;

signals:
    Q_REVISION(1) void accelerationModeChanged(AccelerationMode accelerationMode);

private:
    QAccelerometer *m_sensor;
    QmlSensorReading *createReading() const override;
};

class Q_SENSORSQUICK_EXPORT QmlAccelerometerReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x NOTIFY xChanged BINDABLE bindableX)
    Q_PROPERTY(qreal y READ y NOTIFY yChanged BINDABLE bindableY)
    Q_PROPERTY(qreal z READ z NOTIFY zChanged BINDABLE bindableZ)
    QML_NAMED_ELEMENT(AccelerometerReading)
    QML_UNCREATABLE("Cannot create AccelerometerReading")
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlAccelerometerReading(QAccelerometer *sensor);
    ~QmlAccelerometerReading();

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
    QSensorReading *reading() const  override;
    void readingUpdate() override;
    QAccelerometer *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlAccelerometerReading, qreal,
                               m_x, &QmlAccelerometerReading::xChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlAccelerometerReading, qreal,
                               m_y, &QmlAccelerometerReading::yChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlAccelerometerReading, qreal,
                               m_z, &QmlAccelerometerReading::zChanged)
};

QT_END_NAMESPACE
#endif
