// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2

Item {
    component Nested: Item {
        Component.onCompleted: {
            const outerLoopCounter = 0;
            const innerLoopCounter = 0;
            const additionOperationResult
                  = outerLoopCounter
                  + innerLoopCounter;
            const subtractionOperationResult
                  = outerLoopCounter
                  - innerLoopCounter;
            const multiplicationOperationResult
                  = outerLoopCounter
                  * innerLoopCounter;
            const divisionOperationResult
                  = outerLoopCounter
                  / innerLoopCounter;
            const divisionOperationResult1
                  = outerLoopCounter
                  / innerLoopCounter;
            const modulusOperationResult1
                  = outerLoopCounter
                  % innerLoopCounter;
            const exponentiationOperationResult
                  = outerLoopCounter
                  ** innerLoopCounter;
            const bitwiseAndOperationResult
                  = outerLoopCounter
                  & innerLoopCounter;
            const bitwiseOrOperationResult
                  = outerLoopCounter
                  | innerLoopCounter;
            const bitwiseXorOperationResult
                  = outerLoopCounter
                  ^ innerLoopCounter;
            const leftShiftOperationResult
                  = outerLoopCounter
                  << innerLoopCounter;
            const rightShiftOperationResult
                  = outerLoopCounter
                  >> innerLoopCounter;
            const unsignedRightShiftOperationResult
                  = outerLoopCounter
                  >>> innerLoopCounter;
        }
    }
}
