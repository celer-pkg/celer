// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IOSMAGNETOMETER_H
#define IOSMAGNETOMETER_H

#include <qsensorbackend.h>
#include <qmagnetometer.h>

@class CMMotionManager;

QT_BEGIN_NAMESPACE

class IOSMagnetometer : public QSensorBackend
{
public:
    static char const * const id;

    explicit IOSMagnetometer(QSensor *sensor);
    void timerEvent(QTimerEvent *) override;

    void start() override;
    void stop() override;

    void startMagnetometer();
    void startDeviceMotion();

private:
    CMMotionManager *m_motionManager;
    QMagnetometerReading m_reading;
    int m_timer;
    bool m_returnGeoValues;

    static int s_magnetometerStartCount;
    static int s_deviceMotionStartCount;
};
QT_END_NAMESPACE

#endif // IOSMAGNETOMETER_H

