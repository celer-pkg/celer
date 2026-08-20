// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQuick

QtObject {
    id: uiStyle

    property bool darkMode: (Application.styleHints.colorScheme === Qt.ColorScheme.Dark)

    // Font Sizes
    readonly property int fontSizeXXS: 8
    readonly property int fontSizeXS: 10
    readonly property int fontSizeS: 12
    readonly property int fontSizeM: 16
    readonly property int fontSizeL: 20
    readonly property int fontSizeXL: 24

    // Color Scheme
    readonly property color colorRed: "#E91E63"

    readonly property color buttonGray: darkMode ? "#808080" : "#f3f3f4"
    readonly property color buttonGrayPressed: darkMode ? "#707070" : "#cecfd5"
    readonly property color buttonGrayOutline: darkMode ? "#0D0D0D" : "#999999"

    readonly property color buttonBackground: darkMode ? "#262626" : "#CCCCCC"
    readonly property color buttonPressed: darkMode ? "#1E1E1E" : "#BEBEC4"
    readonly property color buttonOutline: darkMode ? "#0D0D0D" : "#999999"

    readonly property color background: darkMode ? "#262626" : "#E6E6E6"
    readonly property color background1: darkMode ? "#00414A" : "#ceded6"

    readonly property color textOnLightBackground: "#191919"
    readonly property color textOnDarkBackground: "#E6E6E6"

    readonly property color textColor: darkMode ? "#E6E6E6" : "#191919"
    readonly property color titletextColor: darkMode ? "#2CDE85" : "#191919"

    readonly property color highlightColor: darkMode ? "#33676E" : "#28C878"
    readonly property color highlightBorderColor: darkMode ? "#4F8C95" : "#1FA05E"

    function iconPath(baseImagePath) {
        if (darkMode)
            return `qrc:/qt/qml/ColorPalette/icons/${baseImagePath}_dark.svg`
        else
            return `qrc:/qt/qml/ColorPalette/icons/${baseImagePath}.svg`

    }
}
