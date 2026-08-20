// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidrotation.h"

#include <QtCore/qmath.h>

AndroidRotation::AndroidRotation(int type, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QRotationReading>(type, sensor, parent)
{}


void AndroidRotation::dataReceived(const ASensorEvent &event)
{
    // From android documentation, the rotation sensor values are:
    // values[0]: x*sin(θ/2)
    // values[1]: y*sin(θ/2)
    // values[2]: z*sin(θ/2)
    // values[3]: cos(θ/2)

    // The mathematics below is adapted from
    // https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/hardware/SensorManager.java#1644
    // and
    // https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/hardware/SensorManager.java#1469
    // (getRotationMatrixFromVector() followed by getOrientation())
    qreal angles[3];
    qreal q1 = qreal(event.data[0]);
    qreal q2 = qreal(event.data[1]);
    qreal q3 = qreal(event.data[2]);
    qreal q0 = qreal(event.data[3]);

    qreal sq_q1 = 2 * q1 * q1;
    qreal sq_q2 = 2 * q2 * q2;
    qreal sq_q3 = 2 * q3 * q3;
    qreal q1_q2 = 2 * q1 * q2;
    qreal q3_q0 = 2 * q3 * q0;
    qreal q1_q3 = 2 * q1 * q3;
    qreal q2_q0 = 2 * q2 * q0;
    qreal q2_q3 = 2 * q2 * q3;
    qreal q1_q0 = 2 * q1 * q0;

    angles[0] = std::atan2((q1_q2 - q3_q0), (1 - sq_q1 - sq_q3));
    angles[1] = std::asin(-(q2_q3 + q1_q0));
    angles[2] = std::atan2(-(q1_q3 - q2_q0), (1 - sq_q1 - sq_q2));

    qreal rz = -qRadiansToDegrees(angles[0]);
    qreal rx = -qRadiansToDegrees(angles[1]);
    qreal ry =  qRadiansToDegrees(angles[2]);

    if (sensor()->skipDuplicates() && qFuzzyCompare(m_reader.x(), rx) &&
            qFuzzyCompare(m_reader.y(), ry) &&
            qFuzzyCompare(m_reader.z(), rz)) {
        return;
    }
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    m_reader.setFromEuler(rx, ry, rz);
    newReadingAvailable();
}
