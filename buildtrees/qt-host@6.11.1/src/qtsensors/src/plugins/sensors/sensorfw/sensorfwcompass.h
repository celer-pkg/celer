// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef SENSORFWCOMPASS_H
#define SENSORFWCOMPASS_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qcompass.h>

#include <compasssensor_i.h>
#include <datatypes/compass.h>



class SensorfwCompass : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    SensorfwCompass(QSensor *sensor);
protected:
    bool doConnect() override;
    QString sensorName() const override;
    void start() override;
    virtual void init();
private:
    QCompassReading m_reading;
    bool m_initDone;
private slots:
    void slotDataAvailable(const Compass& data);
};

#endif
