// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTS_SPEEDOMETER_H
#define TESTS_SPEEDOMETER_H

#include "rep_speedometer_merged.h"

class Speedometer : public SpeedometerSimpleSource
{
    Q_OBJECT
public:
    Speedometer(QObject *parent = nullptr);
    ~Speedometer() override;

private:
    int speed;
};

#endif
