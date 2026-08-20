// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T
import ToDoList

T.SwitchDelegate {
    id: control

    padding: 6
    spacing: 6

    font.pixelSize: AppSettings.fontSize
    implicitWidth: implicitIndicatorWidth + implicitContentWidth
    implicitHeight: Math.max(implicitIndicatorHeight, implicitContentHeight) + padding

    indicator: Rectangle {
        implicitWidth: 40
        implicitHeight: 20
        x: control.width - implicitWidth
        y: parent.height / 2 - height / 2
        radius: 10
        opacity: 0.75
        color: control.checked ? "#41CD52" : "#ffffff"
        border.color: control.checked ? "#41CD52" : Constants.secondaryColor

        Rectangle {
            x: control.checked ? parent.width - width : 0
            width: 20
            height: 20
            radius: 10
            color: control.down ? "#cccccc" : "#ffffff"
            border.color: control.checked ? "#41CD52" : Constants.secondaryColor
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: Constants.secondaryColor
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.padding
    }
}
