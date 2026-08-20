// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQml
import "calculator.js" as CalcEngine

QtObject {
    required property Display display

    function operatorPressed(operator) {
        CalcEngine.operatorPressed(operator, display);
    }
    function digitPressed(digit) {
        CalcEngine.digitPressed(digit, display);
    }
    function isButtonDisabled(op) {
        return CalcEngine.isOperationDisabled(op, display);
    }
}
