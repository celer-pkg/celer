// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

RoundButton {
    id: button
    implicitWidth: 48
    implicitHeight: 38
    radius: buttonRadius
    icon.source: getIcon()
    icon.width: 38
    icon.height: 38
    icon.color: getIconColor()
    // include this text property as the calculator engine
    // differentiates buttons through text. The text is never drawn.
    text: "bs"
    Accessible.name: "backspace"

    property bool dimmable: true
    property bool dimmed: false
    readonly property color backgroundColor: "#222222"
    readonly property color borderColor: "#A9A9A9"
    readonly property color backspaceRedColor: "#DE2C2C"
    readonly property int buttonRadius: 8

    function getBackgroundColor() {
        if (button.dimmable && button.dimmed)
            return backgroundColor;
        if (button.pressed)
            return backspaceRedColor;
        return backgroundColor;
    }

    function getBorderColor() {
        if (button.dimmable && button.dimmed)
            return borderColor;
        if (button.pressed || button.hovered)
            return backspaceRedColor;
        return borderColor;
    }

    function getIconColor() {
        if (button.dimmable && button.dimmed)
            return Qt.darker(backspaceRedColor);
        if (button.pressed)
            return backgroundColor;
        return backspaceRedColor;
    }

    function getIcon() {
        if (button.dimmable && button.dimmed)
            return "images/backspace.svg";
        if (button.pressed)
            return "images/backspace_fill.svg";
        return "images/backspace.svg";
    }

    background: Rectangle {
        radius: button.buttonRadius
        color: button.getBackgroundColor()
        border.color: button.getBorderColor()
    }
}
