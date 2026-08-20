// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WINRTROTATIONSENSOR_H
#define WINRTROTATIONSENSOR_H

#include <QtSensors/QSensorBackend>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class WinRtRotationSensorPrivate;
class WinRtRotationSensor : public QSensorBackend
{
    Q_OBJECT
public:
    WinRtRotationSensor(QSensor *sensor);
    ~WinRtRotationSensor();

    bool isFeatureSupported(QSensor::Feature feature) const override
    {
        if (feature == QSensor::Feature::AxesOrientation || feature == QSensor::Feature::AccelerationMode)
            return true;
        return false;
    }

    void start() override;
    void stop() override;

private:
    QScopedPointer<WinRtRotationSensorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(WinRtRotationSensor)
};

QT_END_NAMESPACE

#endif // WINRTROTATIONSENSOR_H
