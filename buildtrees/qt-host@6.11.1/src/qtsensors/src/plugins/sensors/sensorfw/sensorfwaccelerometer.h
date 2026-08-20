// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef SENSORFWACCELEROMETER_H
#define SENSORFWACCELEROMETER_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qaccelerometer.h>

#include <accelerometersensor_i.h>
#include <datatypes/xyz.h>



class sensorfwaccelerometer : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    sensorfwaccelerometer(QSensor *sensor);
protected:
    bool doConnect() override;
    QString sensorName() const override;
    qreal correctionFactor() const override;
    void start() override;
    virtual void init();

private:
    QAccelerometerReading m_reading;
    bool m_initDone;

private slots:
    void slotDataAvailable(const XYZ &data);
    void slotFrameAvailable(const QList<XYZ> &);
};

#endif
