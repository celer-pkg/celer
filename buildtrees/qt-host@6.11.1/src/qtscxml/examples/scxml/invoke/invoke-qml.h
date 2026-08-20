// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef INVOKE_QML
#define INVOKE_QML

#include "statemachine.h"

#include <QtQml/qqml.h>
#include <QtCore/qobject.h>

struct DirectionsStateMachineRegistration
{
    Q_GADGET
    QML_FOREIGN(DirectionsStateMachine)
    QML_NAMED_ELEMENT(DirectionsStateMachine)
    QML_ADDED_IN_VERSION(1, 0)
};

#endif // INVOKE_QML
