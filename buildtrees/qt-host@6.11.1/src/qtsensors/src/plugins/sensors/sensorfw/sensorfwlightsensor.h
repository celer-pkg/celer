// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#ifndef SENSORFWLIGHTSENSOR_H
#define SENSORFWLIGHTSENSOR_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qlightsensor.h>

#include <alssensor_i.h>


class SensorfwLightSensor : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    SensorfwLightSensor(QSensor *sensor);
protected:
    bool doConnect() override;
    QString sensorName() const override;
    void start() override;
    virtual void init();
private:
    QLightReading m_reading;
    bool m_initDone;
private slots:
    void slotDataAvailable(const Unsigned& data);
};

#endif
