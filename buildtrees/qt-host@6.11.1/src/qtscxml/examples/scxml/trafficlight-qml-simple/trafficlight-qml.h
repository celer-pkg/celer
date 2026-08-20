// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TRAFFICLIGHT_QML
#define TRAFFICLIGHT_QML

#include "statemachine.h"

#include <QtQml/qqml.h>
#include <QtCore/qobject.h>

struct TrafficLightStateMachineRegistration
{
    Q_GADGET
    QML_FOREIGN(TrafficLightStateMachine)
    QML_NAMED_ELEMENT(TrafficLightStateMachine)
    QML_ADDED_IN_VERSION(1, 0)
};

#endif // TRAFFICLIGHT_QML
