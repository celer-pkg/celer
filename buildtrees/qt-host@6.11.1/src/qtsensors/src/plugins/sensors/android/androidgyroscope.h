// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDGYROSCOPE_H
#define ANDROIDGYROSCOPE_H

#include <qgyroscope.h>

#include "sensoreventqueue.h"

class AndroidGyroscope : public SensorEventQueue<QGyroscopeReading>
{
public:
    AndroidGyroscope(int type, QSensor *sensor, QObject *parent = nullptr);

protected:
    // SensorEventQueue interface
    void dataReceived(const ASensorEvent &event) override;
};

#endif // ANDROIDGYROSCOPE_H
