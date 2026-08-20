// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IOSPROXIMITYSENSOR_H
#define IOSPROXIMITYSENSOR_H

#include <qsensorbackend.h>
#include <qproximitysensor.h>

@class ProximitySensorCallback;

QT_BEGIN_NAMESPACE

class IOSProximitySensor : public QSensorBackend
{
public:
    static char const * const id;

    explicit IOSProximitySensor(QSensor *sensor);
    ~IOSProximitySensor();

    void start() override;
    void stop() override;

    void proximityChanged(bool close);
    static bool available();

private:
    ProximitySensorCallback *m_proximitySensorCallback;
    QProximityReading m_reading;

    static int s_startCount;
};
QT_END_NAMESPACE

#endif // IOSPROXIMITYSENSOR_H

