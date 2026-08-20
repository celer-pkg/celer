// Copyright (C) 2016 Canonical, Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#ifndef SENSORFWLIDSENSOR_H
#define SENSORFWLIDSENSOR_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qlidsensor.h>

#include <lidsensor_i.h>
#include <liddata.h>


class SensorfwLidSensor : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    SensorfwLidSensor(QSensor *sensor);
protected:
    bool doConnect() override;
    QString sensorName() const override;
    void start() override;
    virtual void init();
private:
    QLidReading m_reading;
    bool m_initDone;
private slots:
    void slotDataAvailable(const LidData& data);
};

#endif
