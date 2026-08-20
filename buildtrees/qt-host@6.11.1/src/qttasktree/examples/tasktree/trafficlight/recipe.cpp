// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "recipe.h"

#include "glueinterface.h"

#include <QBarrier>

using namespace QtTaskTree;
using namespace std::chrono;

//! [1]
static Group switchLightsTask(GlueInterface *iface, Lights lights, const milliseconds &timeout)
{
    return {
        onGroupSetup([iface, lights] { iface->setLights(lights); }),
        timeoutTask(timeout, DoneResult::Success)
    };
}
//! [1]

//! [0]
ExecutableItem recipe(GlueInterface &iface)
{
    return Forever {
        Group { // "working" state
            stopOnSuccess,
            parallel,
            signalAwaiterTask(&iface, &GlueInterface::smashed), // transitions to the "broken" state
            Forever {
                switchLightsTask(&iface, Light::Red, 3s), // "red" state
                switchLightsTask(&iface, Light::Red | Light::Yellow, 1s), // "redGoingGreen" state
                switchLightsTask(&iface, Light::Green, 3s), // "green" state
                switchLightsTask(&iface, Light::Yellow, 1s), // "greenGoingRed" state
            }
        },
        Group { // "broken" state
            stopOnSuccess,
            parallel,
            signalAwaiterTask(&iface, &GlueInterface::repaired), // transitions to the "working" state
            Forever {
                switchLightsTask(&iface, Light::Yellow, 1s), // "blinking" state
                switchLightsTask(&iface, Light::Off, 1s), // "unblinking" state
            }
        }
    };
}
//! [0]
