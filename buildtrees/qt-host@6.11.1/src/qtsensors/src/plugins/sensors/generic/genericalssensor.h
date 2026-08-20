// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GENERICALSSENSOR_H
#define GENERICALSSENSOR_H

#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qlightsensor.h>
#include <QtSensors/qambientlightsensor.h>

class genericalssensor : public QSensorBackend, public QLightFilter
{
public:
    static char const * const id;

    genericalssensor(QSensor *sensor);

    void start() override;
    void stop() override;

    bool filter(QLightReading *reading) override;

private:
    QAmbientLightReading m_reading;
    QLightSensor *lightSensor;
};

#endif

