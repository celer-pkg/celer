// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

T.ToolButton {
    id: control

    property alias buttonColor: rect.color
    property alias buttonBorderColor: rect.border.color
    property alias textColor: label.color
    property alias fontBold: label.font.bold

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: 4
    rightPadding: 4
    topPadding: 4
    bottomPadding: 4

    background: Rectangle {
        id: rect
        color: "transparent"
        border.width: 1
        radius: 3
        border.color: control.hovered
                      ? UIStyle.buttonOutline
                      : "transparent"
    }

    icon.width: 15
    icon.height: 15
    icon.color: UIStyle.textColor

    contentItem: IconLabel {
        id: label
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display

        icon: control.icon
        text: control.text
        font.pixelSize: UIStyle.fontSizeS
        color: UIStyle.textColor
    }
}
