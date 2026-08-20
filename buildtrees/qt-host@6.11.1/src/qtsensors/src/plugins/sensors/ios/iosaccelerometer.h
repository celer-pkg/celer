// Copyright (C) 2016 Lorn Potter
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IOSACCELEROMETER_H
#define IOSACCELEROMETER_H

#include <qsensorbackend.h>
#include <qaccelerometer.h>

@class CMMotionManager;

QT_BEGIN_NAMESPACE

class IOSAccelerometer : public QSensorBackend
{
public:
    static char const * const id;

    explicit IOSAccelerometer(QSensor *sensor);
    void timerEvent(QTimerEvent *) override;

    void start() override;
    void stop() override;

private:
    CMMotionManager *m_motionManager;
    QAccelerometerReading m_reading;
    int m_timer;

    static int s_startCount;
};
QT_END_NAMESPACE

#endif // IOSACCELEROMETER_H

