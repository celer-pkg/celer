// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDPRESSURE_H
#define ANDROIDPRESSURE_H

#include <qpressuresensor.h>

#include "sensoreventqueue.h"

class AndroidPressure : public SensorEventQueue<QPressureReading>
{
public:
    AndroidPressure(int type, QSensor *sensor, QObject *parent = nullptr);

protected:
    // SensorEventQueue interface
    void dataReceived(const ASensorEvent &event) override;
};

#endif // ANDROIDPRESSURE_H
