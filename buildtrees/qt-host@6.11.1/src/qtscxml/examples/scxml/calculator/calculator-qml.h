// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CALCULATOR_QML
#define CALCULATOR_QML

#include "statemachine.h"

#include <QtQml/qqml.h>
#include <QtCore/qobject.h>

struct CalculatorStateMachineRegistration
{
    Q_GADGET
    QML_FOREIGN(CalculatorStateMachine)
    QML_NAMED_ELEMENT(CalculatorStateMachine)
    QML_ADDED_IN_VERSION(1, 0)
};

#endif // CALCULATOR_QML
