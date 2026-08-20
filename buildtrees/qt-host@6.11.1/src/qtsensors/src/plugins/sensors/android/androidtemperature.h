// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef ANDROIDTEMPERATURE_H
#define ANDROIDTEMPERATURE_H

#include <qambienttemperaturesensor.h>

#include "sensoreventqueue.h"

class AndroidTemperature : public SensorEventQueue<QAmbientTemperatureReading>
{
public:
    AndroidTemperature(int type, QSensor *sensor, QObject *parent = nullptr);

protected:
    // SensorEventQueue interface
    void dataReceived(const ASensorEvent &event) override;
};

#endif // ANDROIDTEMPERATURE_H
