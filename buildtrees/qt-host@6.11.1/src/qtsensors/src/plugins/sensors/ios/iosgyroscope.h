// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IOSGYROSCOPE_H
#define IOSGYROSCOPE_H

#include <qsensorbackend.h>
#include <qgyroscope.h>

@class CMMotionManager;

QT_BEGIN_NAMESPACE

class IOSGyroscope : public QSensorBackend
{
public:
    static char const * const id;

    explicit IOSGyroscope(QSensor *sensor);
    void timerEvent(QTimerEvent *) override;

    void start() override;
    void stop() override;

private:
    CMMotionManager *m_motionManager;
    QGyroscopeReading m_reading;
    int m_timer;

    static int s_startCount;
};
QT_END_NAMESPACE

#endif // IOSGYROSCOPE_H

