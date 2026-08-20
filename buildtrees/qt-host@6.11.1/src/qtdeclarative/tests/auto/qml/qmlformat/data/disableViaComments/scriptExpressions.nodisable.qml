// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    function doSomething() {
        // qmlformat off
        console.log("This function is not formatted by qmlformat");
        // qmlformat on
        x = 12;
    }

    height: 100
    width: 100

    Rectangle {
        color: "blue"
        height: 50
        width: 50
    }
}
