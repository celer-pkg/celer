// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import ToDoList

Label {
    id: root

    property alias backgroundColor: backgroundRec.color
    property alias backgroundOpacity: backgroundRec.opacity
    property alias backgroundBorder: backgroundRec.border
    property alias tapHandler: tapHandler

    font.pixelSize: AppSettings.fontSize - 4
    leftInset: -8
    rightInset: -8
    topInset: -4
    bottomInset: -4

    color: "#41CD52"
    font.bold: true
    visible: root.text

    background: Rectangle {
        id: backgroundRec

        color: tapHandler.pressed ? "#41CD52" : "transparent"
        opacity: 0.3
        radius: 6
        border.color: "#41CD52"

        TapHandler {
            id: tapHandler
        }
    }
}
