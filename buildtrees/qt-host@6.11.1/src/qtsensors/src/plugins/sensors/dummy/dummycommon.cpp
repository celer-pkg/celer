// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "dummycommon.h"

#include <QtCore/qdeadlinetimer.h>

dummycommon::dummycommon(QSensor *sensor)
    : QSensorBackend(sensor)
    , m_timerid(0)
{
}

void dummycommon::start()
{
    if (m_timerid)
        return;

    int dataRate = sensor()->dataRate();
    if (dataRate == 0) {
        if (sensor()->availableDataRates().size())
            // Use the first available rate when -1 is chosen
            dataRate = sensor()->availableDataRates().first().first;
        else
            dataRate = 1;
    }

    int interval = 1000 / dataRate;

    if (interval)
        m_timerid = startTimer(interval);
}

void dummycommon::stop()
{
    if (m_timerid) {
        killTimer(m_timerid);
        m_timerid = 0;
    }
}

void dummycommon::timerEvent(QTimerEvent * /*event*/)
{
    poll();
}

quint64 dummycommon::getTimestamp()
{
    return QDeadlineTimer::current().deadlineNSecs() / 1000;
}

