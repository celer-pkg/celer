// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
import QtQuick

TextInput {
    focus: true
    validator: IntValidator {
        bottom: 0
        top: 100
    }
    color: acceptableInput ? "black" : "red";
}
//![0]
