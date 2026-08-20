// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Basic

ItemDelegate {
    id: root

    property bool redHover: false
    readonly property color pressedColor: root.redHover ? Theme.buttonRedTextPressedColor
                                                        : Theme.buttonTextPressedColor

    contentItem: Text {
        text: root.text
        font.pixelSize: Theme.mediumFontSize
        font.weight: Theme.fontDefaultWeight
        color: root.pressed ? root.pressedColor : Theme.buttonTextColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
        border {
            width: 2
            color: root.pressed ? root.pressedColor : Theme.buttonTextColor
        }
        radius: 10
        color: Theme.buttonBackgroundColor
    }
}
