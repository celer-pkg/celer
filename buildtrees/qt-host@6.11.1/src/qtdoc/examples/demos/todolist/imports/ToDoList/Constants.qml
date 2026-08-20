// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQuick

QtObject {
    id: root

    readonly property int width: 360
    readonly property int height: 800

    property bool isDarkModeActive: AppSettings.theme == "Dark"
    readonly property color mainColor: isDarkModeActive ? backgroundDarkColor : backgroundLightColor
    readonly property color secondaryColor: isDarkModeActive ? backgroundLightColor : backgroundDarkColor

    readonly property string clockIconSource: isDarkModeActive ? "images/Clock.svg" : "images/Clock_Dark.svg"
    readonly property string minClockIconSource: isDarkModeActive ? "images/Minutes_Clock.svg" : "images/Minutes_Clock_Dark.svg"

    readonly property color backgroundLightColor: "#F3F3F4"
    readonly property color backgroundDarkColor: "#09102B"
}
