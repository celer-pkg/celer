// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDMAGNETOMETER_H
#define ANDROIDMAGNETOMETER_H

#include <qmagnetometer.h>

#include "sensoreventqueue.h"

class AndroidMagnetometer : public SensorEventQueue<QMagnetometerReading>
{
public:
    AndroidMagnetometer(int type, QSensor *sensor, QObject *parent = nullptr);

protected:
    // SensorEventQueue interface
    void dataReceived(const ASensorEvent &event) override;
};

#endif // ANDROIDMAGNETOMETER_H
