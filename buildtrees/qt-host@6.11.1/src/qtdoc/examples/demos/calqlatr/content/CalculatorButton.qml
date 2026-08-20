// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

RoundButton {
    id: button
    implicitWidth: 38
    implicitHeight: 38
    radius: buttonRadius

    property bool dimmable: false
    property bool dimmed: false
    readonly property int fontSize: 22
    readonly property int buttonRadius: 8
    property color textColor: "#FFFFFF"
    property color accentColor: "#2CDE85"
    readonly property color backgroundColor: "#222222"
    readonly property color borderColor: "#A9A9A9"

    function getBackgroundColor() {
        if (button.dimmable && button.dimmed)
            return backgroundColor;
        if (button.pressed)
            return accentColor;
        return backgroundColor;
    }

    function getBorderColor() {
        if (button.dimmable && button.dimmed)
            return borderColor;
        if (button.pressed || button.hovered)
            return accentColor;
        return borderColor;
    }

    function getTextColor() {
        if (button.dimmable && button.dimmed)
            return Qt.darker(textColor);
        if (button.pressed)
            return backgroundColor;
        if (button.hovered)
            return accentColor;
        return textColor;
    }

    background: Rectangle {
        radius: button.buttonRadius
        color: button.getBackgroundColor()
        border.color: button.getBorderColor()
    }

    contentItem: Text {
        text: button.text
        font.pixelSize: button.fontSize
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: button.getTextColor()
        Behavior on color {
            ColorAnimation {
                duration: 120
                easing.type: Easing.OutElastic
            }
        }
    }
}
