// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl
import ToDoList

T.ToolButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)
    padding: 12
    spacing: 6

    font.pixelSize: 12
    icon.color: control.hovered ? Constants.mainColor : Constants.secondaryColor

    contentItem: IconLabel {
        spacing: control.spacing
        icon: control.icon
        font: control.font
        text: control.text
        color: Constants.secondaryColor
    }

    background: Rectangle {
        color: "#4caf50"
    }
}
