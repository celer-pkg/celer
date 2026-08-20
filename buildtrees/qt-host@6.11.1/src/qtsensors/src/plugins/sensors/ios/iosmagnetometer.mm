// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iosmotionmanager.h"
#include "iosmagnetometer.h"

#import <CoreMotion/CoreMotion.h>

QT_BEGIN_NAMESPACE

char const * const IOSMagnetometer::id("ios.magnetometer");

int IOSMagnetometer::s_magnetometerStartCount = 0;
int IOSMagnetometer::s_deviceMotionStartCount = 0;

IOSMagnetometer::IOSMagnetometer(QSensor *sensor)
    : QSensorBackend(sensor)
    , m_motionManager([QIOSMotionManager sharedManager])
    , m_timer(0)
    , m_returnGeoValues(true)
{
    setReading<QMagnetometerReading>(&m_reading);
    // Technical information about data rate is not found, but
    // seems to be ~70Hz after testing on iPad4:
    addDataRate(1, 70);
    // Output range is +/- 2 gauss (0.0002 tesla) and can sense magnetic fields less than
    // 100 microgauss (1e-08 tesla) Ref: "iOS Sensor Programming", Alasdair, 2012.
    addOutputRange(-0.0002, 0.0002, 1e-08);
}

void IOSMagnetometer::start()
{
    if (m_timer != 0)
        return;

    int hz = sensor()->dataRate();
    m_timer = startTimer(1000 / (hz == 0 ? 60 : hz));
    m_returnGeoValues = static_cast<QMagnetometer *>(sensor())->returnGeoValues();

    if (m_returnGeoValues) {
        if (++s_deviceMotionStartCount == 1)
            [m_motionManager startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXMagneticNorthZVertical];
    } else {
        if (++s_magnetometerStartCount == 1)
            [m_motionManager startMagnetometerUpdates];
    }
}

void IOSMagnetometer::stop()
{
    if (m_timer == 0)
        return;

    killTimer(m_timer);
    m_timer = 0;

    if (m_returnGeoValues) {
        if (--s_deviceMotionStartCount == 0)
            [m_motionManager stopDeviceMotionUpdates];
    } else {
        if (--s_magnetometerStartCount == 0)
            [m_motionManager stopMagnetometerUpdates];
    }
}

void IOSMagnetometer::timerEvent(QTimerEvent *)
{
    CMMagneticField field;

    if (m_returnGeoValues) {
        CMDeviceMotion *deviceMotion = m_motionManager.deviceMotion;
        CMCalibratedMagneticField calibratedField = deviceMotion.magneticField;
        field = calibratedField.field;
        // skip update if NaN
        if (field.x != field.x || field.y != field.y || field.z != field.z)
            return;
        m_reading.setTimestamp(quint64(deviceMotion.timestamp * 1e6));

        switch (calibratedField.accuracy) {
        case CMMagneticFieldCalibrationAccuracyUncalibrated:
            m_reading.setCalibrationLevel(0.0);
            break;
        case CMMagneticFieldCalibrationAccuracyLow:
            m_reading.setCalibrationLevel(0.3);
            break;
        case CMMagneticFieldCalibrationAccuracyMedium:
            m_reading.setCalibrationLevel(0.6);
            break;
        case CMMagneticFieldCalibrationAccuracyHigh:
            m_reading.setCalibrationLevel(1.0);
            break;
        }
    } else {
        CMMagnetometerData *data = m_motionManager.magnetometerData;
        field = data.magneticField;
        // skip update if NaN
        if (field.x != field.x || field.y != field.y || field.z != field.z)
            return;
        m_reading.setTimestamp(quint64(data.timestamp * 1e6));
        m_reading.setCalibrationLevel(1.0);
    }

    // Convert NSTimeInterval to microseconds and microtesla to tesla:
    m_reading.setX(qreal(field.x) / 1e6);
    m_reading.setY(qreal(field.y) / 1e6);
    m_reading.setZ(qreal(field.z) / 1e6);
    newReadingAvailable();
}

QT_END_NAMESPACE
