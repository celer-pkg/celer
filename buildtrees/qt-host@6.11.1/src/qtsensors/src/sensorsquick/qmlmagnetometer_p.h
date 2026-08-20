// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLMAGNETOMETER_P_H
#define QMLMAGNETOMETER_P_H

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

class QMagnetometer;

class Q_SENSORSQUICK_EXPORT QmlMagnetometer : public QmlSensor
{
    Q_OBJECT
    Q_PROPERTY(bool returnGeoValues READ returnGeoValues WRITE setReturnGeoValues NOTIFY returnGeoValuesChanged)
    QML_NAMED_ELEMENT(Magnetometer)
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlMagnetometer(QObject *parent = 0);
    ~QmlMagnetometer();

    bool returnGeoValues() const;
    void setReturnGeoValues(bool geo);

    QSensor *sensor() const override;

Q_SIGNALS:
    void returnGeoValuesChanged(bool returnGeoValues);

private:
    QMagnetometer *m_sensor;
    QmlSensorReading *createReading() const override;
};

class Q_SENSORSQUICK_EXPORT QmlMagnetometerReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x NOTIFY xChanged BINDABLE bindableX)
    Q_PROPERTY(qreal y READ y NOTIFY yChanged BINDABLE bindableY)
    Q_PROPERTY(qreal z READ z NOTIFY zChanged BINDABLE bindableZ)
    Q_PROPERTY(qreal calibrationLevel READ calibrationLevel
               NOTIFY calibrationLevelChanged BINDABLE bindableCalibrationLevel)
    QML_NAMED_ELEMENT(MagnetometerReading)
    QML_UNCREATABLE("Cannot create MagnetometerReading")
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlMagnetometerReading(QMagnetometer *sensor);
    ~QmlMagnetometerReading();

    qreal x() const;
    QBindable<qreal> bindableX() const;
    qreal y() const;
    QBindable<qreal> bindableY() const;
    qreal z() const;
    QBindable<qreal> bindableZ() const;
    qreal calibrationLevel() const;
    QBindable<qreal> bindableCalibrationLevel() const;


Q_SIGNALS:
    void xChanged();
    void yChanged();
    void zChanged();
    void calibrationLevelChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;
    QMagnetometer *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlMagnetometerReading, qreal,
                               m_x, &QmlMagnetometerReading::xChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlMagnetometerReading, qreal,
                               m_y, &QmlMagnetometerReading::yChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlMagnetometerReading, qreal,
                               m_z, &QmlMagnetometerReading::zChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlMagnetometerReading, qreal,
                               m_calibrationLevel, &QmlMagnetometerReading::calibrationLevelChanged)
};

QT_END_NAMESPACE
#endif
