// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WINRTGYROSCOPE_H
#define WINRTGYROSCOPE_H

#include <QtSensors/QSensorBackend>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class WinRtGyroscopePrivate;
class WinRtGyroscope : public QSensorBackend
{
    Q_OBJECT
public:
    WinRtGyroscope(QSensor *sensor);
    ~WinRtGyroscope();

    bool isFeatureSupported(QSensor::Feature feature) const override
    {
        if (feature == QSensor::Feature::AxesOrientation || feature == QSensor::Feature::AccelerationMode)
            return true;
        return false;
    }

    void start() override;
    void stop() override;

private:
    QScopedPointer<WinRtGyroscopePrivate> d_ptr;
    Q_DECLARE_PRIVATE(WinRtGyroscope)
};

QT_END_NAMESPACE

#endif // WINRTGYROSCOPE_H
