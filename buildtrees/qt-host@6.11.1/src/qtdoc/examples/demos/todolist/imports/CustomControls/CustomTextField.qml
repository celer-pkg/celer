// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import ToDoList

Column {
    id: root

    property alias iconButton : iconButton
    property alias iconSource : icon.source
    property alias placeholderText: textField.placeholderText
    property alias inputMethodHints: textField.inputMethodHints
    property alias validator: textField.validator
    property alias text: textField.text
    property alias headerText : header.text

    spacing: 10

    Label {
        id: header

        text: "title"
        font.pixelSize: AppSettings.fontSize - 2
        color: "#41CD52"
    }

    Item {
        width: root.width
        height: textField.height

        TextField {
            id: textField

            width: parent.width
            //height: root.height - header.height - spacing
        }

        Image {
            id: icon

            anchors.right: textField.right
            anchors.rightMargin: 12
            anchors.verticalCenter: textField.verticalCenter

            TapHandler {
                id: iconButton

                gesturePolicy: TapHandler.ReleaseWithinBounds
            }
        }
    }
}
