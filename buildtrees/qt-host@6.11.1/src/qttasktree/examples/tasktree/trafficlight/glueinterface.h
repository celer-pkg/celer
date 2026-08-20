// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GLUEINTERFACE_H
#define GLUEINTERFACE_H

#include <QObject>

//! [0]
enum class Light
{
    Off    = 0,
    Red    = 1 << 0,
    Yellow = 1 << 1,
    Green  = 1 << 2,
};

Q_DECLARE_FLAGS(Lights, Light)
Q_DECLARE_OPERATORS_FOR_FLAGS(Lights)

class GlueInterface final : public QObject
{
    Q_OBJECT

public:
    // operational logic -> GUI
    void setLights(Lights lights) { emit lightsChanged(lights); }

    // GUI -> operational logic
    void smash() { emit smashed(); }
    void repair() { emit repaired(); }

signals:
    void lightsChanged(Lights lights);

    void smashed();
    void repaired();
};
//! [0]

#endif // GLUEINTERFACE_H
