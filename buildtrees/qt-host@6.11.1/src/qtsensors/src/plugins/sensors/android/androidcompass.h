// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDCOMPASS_H
#define ANDROIDCOMPASS_H

#include <QMutex>
#include <qcompass.h>

#include "sensoreventqueue.h"

class AndroidCompass : public ThreadSafeSensorBackend
{
    Q_OBJECT

public:
    AndroidCompass(QSensor *sensor, QObject *parent = nullptr);
    ~AndroidCompass() override;

    void start() override;
    void stop() override;
private:
    void readAllEvents();
    static int looperCallback(int /*fd*/, int /*events*/, void* data);

private:
    QCompassReading m_reading;
    const ASensor *m_accelerometer = nullptr;
    const ASensor *m_magnetometer = nullptr;
    ASensorEventQueue* m_sensorEventQueue = nullptr;
    ASensorVector m_accelerometerEvent;
    ASensorVector m_magneticEvent;
    QMutex m_sensorsMutex;
};

#endif // ANDROIDCOMPASS_H
