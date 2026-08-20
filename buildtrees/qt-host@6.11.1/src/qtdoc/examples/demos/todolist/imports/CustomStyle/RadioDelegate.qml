// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T
import ToDoList

T.RadioDelegate {
    id: control

    text: qsTr("RadioDelegate")
    checked: true

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    padding: 6
    spacing: 6

    font.pixelSize: AppSettings.fontSize
    font.bold: control.checked

    contentItem: Text {
        leftPadding: control.padding
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: control.checked ? "#17a81a" : Constants.secondaryColor
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: control.width - control.padding - implicitWidth
        y: parent.height / 2 - height / 2
        radius: 9
        color: "transparent"
        border.color: control.checked ? "#17a81a" : Constants.secondaryColor

        Rectangle {
            width: 10
            height: 10
            x: 4
            y: 4
            radius: 5
            color: control.down ? "#17a81a" : "#21be2b"
            visible: control.checked
        }
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        visible: control.down || control.highlighted
        opacity: 0.75
        color: control.down ? "#bdbebf" : "transparent"
    }
}
