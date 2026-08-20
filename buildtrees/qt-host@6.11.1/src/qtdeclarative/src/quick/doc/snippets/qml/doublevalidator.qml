// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
import QtQuick

TextInput {
    focus: true
    validator: DoubleValidator {
        bottom: 0.00
        top: 100.00
        decimals: 2
        notation: DoubleValidator.StandardNotation
    }
    onAcceptableInputChanged:
        color = acceptableInput ? "black" : "red";
}
//![0]
