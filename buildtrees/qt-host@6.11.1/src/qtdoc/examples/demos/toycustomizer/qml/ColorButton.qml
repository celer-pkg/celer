// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

RoundButton {
    id: colorButton
    checkable: true
    property color buttonColor: "transparent"
    property string label: ""

    background: Item {}

    Rectangle {
        id: buttonRect
        anchors.centerIn: parent
        width: ApplicationConfig.responsiveSize(64)
        height: width
        radius: width/2

        color: colorButton.checked ? colorButton.buttonColor : "transparent"
        border.color: colorButton.buttonColor
        border.width: ApplicationConfig.responsiveSize(4)
    }

    ColorIcon {
        id: checkIcon
        implicitWidth: ApplicationConfig.responsiveSize(32)
        implicitHeight: ApplicationConfig.responsiveSize(32)
        source: "icons/check.svg"
        visible: colorButton.checked ? true : false
        anchors.centerIn: parent
    }

    ToyLabel {
        id: labelText
        text: colorButton.label
        textStyle: ApplicationConfig.TextStyle.Button_S
        anchors.top: buttonRect.bottom
        anchors.horizontalCenter: colorButton.horizontalCenter
    }
}
