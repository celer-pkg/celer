// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MYBACKEND_H
#define MYBACKEND_H

#include <qaccelerometer.h>
#include <qsensorbackend.h>

class MyBackend : public QSensorBackend
{
public:
    MyBackend(QSensor *sensor) : QSensorBackend(sensor) {}
    void stop() override {}
    void start() override {}
    void poll() {}

    static const char *id;
};

#endif
