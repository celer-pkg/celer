// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WINRTORIENTATIONSENSOR_H
#define WINRTORIENTATIONSENSOR_H

#include <QtSensors/QSensorBackend>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class WinRtOrientationSensorPrivate;
class WinRtOrientationSensor : public QSensorBackend
{
    Q_OBJECT
public:
    WinRtOrientationSensor(QSensor *sensor);
    ~WinRtOrientationSensor();

    void start() override;
    void stop() override;

private:
    QScopedPointer<WinRtOrientationSensorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(WinRtOrientationSensor)
};

QT_END_NAMESPACE

#endif // WINRTORIENTATIONSENSOR_H
