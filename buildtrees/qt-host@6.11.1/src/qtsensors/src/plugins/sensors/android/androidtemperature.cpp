// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidtemperature.h"

AndroidTemperature::AndroidTemperature(int type, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QAmbientTemperatureReading>(type, sensor, parent)
{}

void AndroidTemperature::dataReceived(const ASensorEvent &event)
{
    if (sensor()->skipDuplicates() && qFuzzyCompare(m_reader.temperature(), qreal(event.temperature)))
        return;
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    // https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_ambient_temperature:
    m_reader.setTemperature(qreal(event.temperature)); // in  degree Celsius
    newReadingAvailable();
}
