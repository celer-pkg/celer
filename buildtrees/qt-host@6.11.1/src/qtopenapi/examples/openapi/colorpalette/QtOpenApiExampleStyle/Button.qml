// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

T.Button {
    id: control

    property alias buttonColor: rect.color
    property alias buttonBorderColor: rect.border.color
    property alias textColor: label.color

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: 15
    rightPadding: 15
    topPadding: 10
    bottomPadding: 10

    background: Rectangle {
        id: rect
        radius: 8
        border.color: UIStyle.buttonOutline
        border.width: 1
        color: UIStyle.buttonBackground
    }

    icon.width: 24
    icon.height: 24
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
