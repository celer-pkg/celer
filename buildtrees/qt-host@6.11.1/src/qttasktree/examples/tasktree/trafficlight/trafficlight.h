// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <QWidget>

class GlueInterface;

class TrafficLight final : public QWidget
{
public:
    TrafficLight(GlueInterface &iface);
};

#endif // TRAFFICLIGHT_H
