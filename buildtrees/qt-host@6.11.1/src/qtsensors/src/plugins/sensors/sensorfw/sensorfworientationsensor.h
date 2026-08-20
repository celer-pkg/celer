// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SENSORFWORIENTATIONSENSOR_H
#define SENSORFWORIENTATIONSENSOR_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qorientationsensor.h>

#include <orientationsensor_i.h>



class SensorfwOrientationSensor : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    SensorfwOrientationSensor(QSensor *sensor);
protected:
    bool doConnect() override;
    QString sensorName() const override;
    void start() override;
    virtual void init();
private:
    QOrientationReading m_reading;
    static QOrientationReading::Orientation getOrientation(int orientation);
    bool m_initDone;

private slots:
    void slotDataAvailable(const Unsigned& orientation);
};

#endif
