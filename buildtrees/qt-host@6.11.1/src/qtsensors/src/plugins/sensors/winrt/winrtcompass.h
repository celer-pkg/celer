// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WINRTCOMPASS_H
#define WINRTCOMPASS_H

#include <QtSensors/QSensorBackend>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class WinRtCompassPrivate;
class WinRtCompass : public QSensorBackend
{
    Q_OBJECT
public:
    WinRtCompass(QSensor *sensor);
    ~WinRtCompass();

    void start() override;
    void stop() override;

private:
    QScopedPointer<WinRtCompassPrivate> d_ptr;
    Q_DECLARE_PRIVATE(WinRtCompass)
};

QT_END_NAMESPACE

#endif // WINRTCOMPASS_H
