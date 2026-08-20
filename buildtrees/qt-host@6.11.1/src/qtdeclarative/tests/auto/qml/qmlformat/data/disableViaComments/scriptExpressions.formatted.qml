// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    width: 100
    height: 100

    function doSomething() {
    // qmlformat off
                console.log("This function is not formatted by qmlformat")
    // qmlformat on
        x = 12;
    }

    Rectangle {
        width: 50
        height: 50
        color: "blue"
    }
}
