// Copyright (C) 2016 BogDan Vatra <bogdan@kde.org>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidproximity.h"

AndroidProximity::AndroidProximity(int type, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QProximityReading>(type, sensor, parent)
{
    m_maximumRange = m_sensorManager->getMaximumRange(m_sensor);

    // if we can't get the range, we arbitrarily define anything closer than 10 cm as "close"
    if (m_maximumRange <= 0)
        m_maximumRange = 10.0;
}


void AndroidProximity::dataReceived(const ASensorEvent &event)
{
    // https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_proximity:
    bool close = qreal(event.distance) < m_maximumRange;
    if (sensor()->skipDuplicates() && close == m_reader.close())
        return;
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    m_reader.setClose(close);
    newReadingAvailable();
}
