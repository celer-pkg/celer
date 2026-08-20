// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#ifndef SENSORFWGYROSCOPE_H
#define SENSORFWGYROSCOPE_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qgyroscope.h>
#include <datatypes/xyz.h>
#include <gyroscopesensor_i.h>




class SensorfwGyroscope : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    SensorfwGyroscope(QSensor *sensor);
protected:
    bool doConnect() override;
    QString sensorName() const override;
    qreal correctionFactor() const override;
    void start() override;
    virtual void init();

private:
    QGyroscopeReading m_reading;
    bool m_initDone;
    static const float MILLI;
private slots:
    void slotDataAvailable(const XYZ &data);
    void slotFrameAvailable(const QList<XYZ> &);
};


#endif // sensorfwGYROSCOPE_H
