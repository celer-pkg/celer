// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IOSPRESSURE_H
#define IOSPRESSURE_H

#include <qsensorbackend.h>
#include <qpressuresensor.h>

@class CMAltimeter;

QT_BEGIN_NAMESPACE

class IOSPressure : public QSensorBackend
{
public:
    static char const * const id;

    explicit IOSPressure(QSensor *sensor);
    ~IOSPressure();
    void timerEvent(QTimerEvent *) override;

    void start() override;
    void stop() override;

private:
    Q_DISABLE_COPY_MOVE(IOSPressure)
    CMAltimeter *m_altimeter = nullptr;
    QPressureReading m_reading;
    int m_timer = 0;

    static int s_startCount;
};
QT_END_NAMESPACE

#endif // IOSPRESSURE_H

