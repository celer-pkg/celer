// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLTAPSENSOR_P_H
#define QMLTAPSENSOR_P_H

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
#include <QtSensors/QTapSensor>

QT_BEGIN_NAMESPACE

class QTapSensor;

class Q_SENSORSQUICK_EXPORT QmlTapSensor : public QmlSensor
{
    Q_OBJECT
    Q_PROPERTY(bool returnDoubleTapEvents READ returnDoubleTapEvents WRITE setReturnDoubleTapEvents NOTIFY returnDoubleTapEventsChanged)
    QML_NAMED_ELEMENT(TapSensor)
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlTapSensor(QObject *parent = 0);
    ~QmlTapSensor();

    bool returnDoubleTapEvents() const;
    void setReturnDoubleTapEvents(bool ret);

    QSensor *sensor() const override;

Q_SIGNALS:
    void returnDoubleTapEventsChanged(bool returnDoubleTapEvents);

private:
    QTapSensor *m_sensor;
    QmlSensorReading *createReading() const override;
};

class Q_SENSORSQUICK_EXPORT QmlTapSensorReading : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(QTapReading::TapDirection tapDirection READ tapDirection
               NOTIFY tapDirectionChanged BINDABLE bindableTapDirection)
    Q_PROPERTY(bool doubleTap READ isDoubleTap
               NOTIFY isDoubleTapChanged BINDABLE bindableDoubleTap)
    QML_NAMED_ELEMENT(TapReading)
    QML_UNCREATABLE("Cannot create TapReading")
    QML_ADDED_IN_VERSION(5,0)
public:

    explicit QmlTapSensorReading(QTapSensor *sensor);
    ~QmlTapSensorReading();

    QTapReading::TapDirection tapDirection() const;
    QBindable<QTapReading::TapDirection> bindableTapDirection() const;
    bool isDoubleTap() const;
    QBindable<bool> bindableDoubleTap() const;

Q_SIGNALS:
    void tapDirectionChanged();
    void isDoubleTapChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;
    QTapSensor *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY(QmlTapSensorReading, QTapReading::TapDirection,
                               m_tapDirection, &QmlTapSensorReading::tapDirectionChanged)
    Q_OBJECT_BINDABLE_PROPERTY(QmlTapSensorReading, bool,
                               m_isDoubleTap, &QmlTapSensorReading::isDoubleTapChanged)
};

QT_END_NAMESPACE
#endif
