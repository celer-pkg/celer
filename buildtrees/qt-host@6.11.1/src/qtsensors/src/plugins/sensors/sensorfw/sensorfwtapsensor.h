// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef SENSORFWTAPSENSOR_H
#define SENSORFWTAPSENSOR_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qtapsensor.h>

#include <tapsensor_i.h>
#include <datatypes/tap.h>



class SensorfwTapSensor : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    SensorfwTapSensor(QSensor *sensor);
protected:
    bool doConnect() override;
    void start() override;
    QString sensorName() const override;
    virtual void init();
private:
    QTapReading m_reading;
    bool m_initDone;
    bool m_isDoubleTapSensor;
    bool m_isOnceStarted;
private slots:
    void slotDataAvailable(const Tap&);
};

#endif
