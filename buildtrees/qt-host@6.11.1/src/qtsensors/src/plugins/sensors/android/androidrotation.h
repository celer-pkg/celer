// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDROTATION_H
#define ANDROIDROTATION_H

#include <qrotationsensor.h>

#include "sensoreventqueue.h"

class AndroidRotation : public SensorEventQueue<QRotationReading>
{
public:
    AndroidRotation(int type, QSensor *sensor, QObject *parent = nullptr);

protected:
    // SensorEventQueue interface
    void dataReceived(const ASensorEvent &event) override;
};

#endif // ANDROIDROTATION_H
