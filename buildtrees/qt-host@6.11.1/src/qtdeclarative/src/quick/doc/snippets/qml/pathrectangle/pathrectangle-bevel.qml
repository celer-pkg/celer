// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Shapes

Window {
    width: rectangleShape.width + 10
    height: rectangleShape.height + 10
    visible: true
    flags: Qt.FramelessWindowHint

//! [shape]
    Shape {
        id: rectangleShape
        width: 200
        height: 150
        anchors.centerIn: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            fillColor: "#3ad23c"

            PathRectangle {
                width: rectangleShape.width
                height: rectangleShape.height
                topLeftRadius: 30
                bottomRightRadius: 30
                bevel: true
            }
        }
    }
//! [shape]
}
