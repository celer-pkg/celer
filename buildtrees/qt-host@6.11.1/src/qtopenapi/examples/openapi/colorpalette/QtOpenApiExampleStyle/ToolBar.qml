// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T

T.ToolBar {
    id: control

    implicitHeight: 25
    spacing: 8

    background: Rectangle {
        color: UIStyle.buttonBackground
        Rectangle {
            height: 1
            width: parent.width
            anchors.top: parent.top
            anchors.left: parent.left
            color: UIStyle.buttonOutline
        }
        Rectangle {
            height: 1
            width: parent.width
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            color: UIStyle.buttonOutline
        }
    }
}
