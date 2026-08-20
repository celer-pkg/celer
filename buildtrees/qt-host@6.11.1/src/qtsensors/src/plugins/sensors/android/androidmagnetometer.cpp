// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidmagnetometer.h"

AndroidMagnetometer::AndroidMagnetometer(int type, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QMagnetometerReading>(type, sensor, parent)
{}

void AndroidMagnetometer::dataReceived(const ASensorEvent &event)
{
    const auto &mag = event.magnetic;
    qreal accuracy = mag.status == ASENSOR_STATUS_NO_CONTACT ? 0 : mag.status / 3.0;
    // check https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_magnetic_field:
    // Android uses micro-Tesla, Qt uses Tesla
    qreal x = qreal(mag.x) / 1e6;
    qreal y = qreal(mag.y) / 1e6;
    qreal z = qreal(mag.z) / 1e6;
    if (sensor()->skipDuplicates() && qFuzzyCompare(accuracy, m_reader.calibrationLevel()) &&
            qFuzzyCompare(x, m_reader.x()) &&
            qFuzzyCompare(y, m_reader.y()) &&
            qFuzzyCompare(z, m_reader.z())) {
        return;
    }
    m_reader.setCalibrationLevel(accuracy);
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    m_reader.setX(x);
    m_reader.setY(y);
    m_reader.setZ(z);
    newReadingAvailable();
}
