// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SENSORFWALS_H
#define SENSORFWALS_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qambientlightsensor.h>

#include <alssensor_i.h>



class Sensorfwals : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    Sensorfwals(QSensor *sensor);
protected:
    bool doConnect() override;
    QString sensorName() const override;
    void start() override;
    virtual void init();

private:
    QAmbientLightReading m_reading;
    bool m_initDone;
private slots:
    void slotDataAvailable(const Unsigned& data);
    static QAmbientLightReading::LightLevel getLightLevel(int lux);

};

#endif
