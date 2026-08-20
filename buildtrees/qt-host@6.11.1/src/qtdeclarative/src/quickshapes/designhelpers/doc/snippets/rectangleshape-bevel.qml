// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Shapes

Window {
    width: rectangleShape.width + 10
    height: rectangleShape.height + 10
    visible: true
    flags: Qt.FramelessWindowHint

//! [rectangleShape]
    RectangleShape {
        id: rectangleShape
        anchors.centerIn: parent
        radius: 0
        topLeftRadius: 30
        bottomRightRadius: 30
        bevel: true
        joinStyle: ShapePath.MiterJoin
        fillColor: "#3ad23c"
        strokeColor: "transparent"
        visible: false
    }
//! [rectangleShape]
}
