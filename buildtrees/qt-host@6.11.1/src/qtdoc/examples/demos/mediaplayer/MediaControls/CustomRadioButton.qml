// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Fusion
import Config

RadioButton {
    id: control

    height: 20

    indicator: Rectangle {
        width: 20
        height: 20
        radius: 10
        x: control.leftPadding
        y: control.height / 2 - height / 2
        color: "transparent"
        border.color: Config.secondaryColor
        border.width: 2

        Rectangle {
            width: 10
            height: 10
            radius: 5
            color: Config.secondaryColor
            anchors.centerIn: parent
            visible: control.checked
        }
    }

    contentItem: Label {
        text: control.text
        font.pixelSize: 18
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
        color: Config.secondaryColor
    }
}
