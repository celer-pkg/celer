// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidpressure.h"

AndroidPressure::AndroidPressure(int type, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QPressureReading>(type, sensor, parent)
{}


void AndroidPressure::dataReceived(const ASensorEvent &event)
{
    // check https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_pressure:
    auto pressurePa = qreal(event.pressure) * 100;
    if (sensor()->skipDuplicates() && qFuzzyCompare(pressurePa, m_reader.pressure()))
        return;
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    m_reader.setPressure(pressurePa); //Android uses hPa, we use Pa
    newReadingAvailable();
}
