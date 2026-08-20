// Copyright (C) 2016 BogDan Vatra <bogdan@kde.org>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDPROXIMITY_H
#define ANDROIDPROXIMITY_H
#include <qproximitysensor.h>

#include "sensoreventqueue.h"

class AndroidProximity : public SensorEventQueue<QProximityReading>
{
public:
    AndroidProximity(int type, QSensor *sensor, QObject *parent = nullptr);

protected:
    // SensorEventQueue interface
    void dataReceived(const ASensorEvent &event) override;

private:
    qreal m_maximumRange;
};

#endif // ANDROIDPROXIMITY_H
