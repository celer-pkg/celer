// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T
import ToDoList

T.ToolTip {
    id: control

    x: parent ? (parent.width - implicitWidth - leftMargin - rightMargin) / 2 : 0
    y: parent ? (parent.height - implicitHeight) / 2 : 0

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    margins: 6
    padding: 12

    font.pixelSize: 14

    closePolicy: T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent

    contentItem: Text {
        text: control.text
        font: control.font
        wrapMode: Text.Wrap
        color: Constants.mainColor
    }

    background: Rectangle {
        border.color: Constants.mainColor
        color: Constants.secondaryColor
        opacity: 0.8
    }
}
