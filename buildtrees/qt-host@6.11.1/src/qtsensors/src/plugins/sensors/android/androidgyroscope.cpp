// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidgyroscope.h"
#include <QtCore/qmath.h>

AndroidGyroscope::AndroidGyroscope(int type, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QGyroscopeReading>(type, sensor, parent)
{}

void AndroidGyroscope::dataReceived(const ASensorEvent &event)
{
    // check https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_gyroscope:
    const auto &vec = event.vector;
    qreal x = qRadiansToDegrees(qreal(vec.x));
    qreal y = qRadiansToDegrees(qreal(vec.y));
    qreal z = qRadiansToDegrees(qreal(vec.z));
    if (sensor()->skipDuplicates() && qFuzzyCompare(m_reader.x(), x) &&
            qFuzzyCompare(m_reader.y(), y) &&
            qFuzzyCompare(m_reader.z(), z)) {
        return;
    }
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    m_reader.setX(x);
    m_reader.setY(y);
    m_reader.setZ(z);
    newReadingAvailable();
}
