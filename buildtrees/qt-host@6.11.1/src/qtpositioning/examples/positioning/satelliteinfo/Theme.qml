// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQuick

QtObject {
    id: root

    property bool darkMode: true

    // colors
    readonly property color greenColor: "#2cde85"
    readonly property color redColor: "#c50000"
    readonly property color whiteColor: "#ffffff"
    readonly property color almostWhiteColor: "#fefefe"
    readonly property color lightGrayColor: "#e8e8e8"
    readonly property color grayColor: "#969696"
    readonly property color darkGrayColor: "#585757"
    readonly property color blackColor: "#000000"

    readonly property color inUseColor: root.greenColor
    readonly property color inViewColor: "#e3a5e5"

    readonly property color iconNormal: root.darkMode ? root.lightGrayColor : root.darkGrayColor
    readonly property color iconSelected: root.greenColor

    readonly property color iconTextNormal: root.darkMode ? root.lightGrayColor
                                                          : root.darkGrayColor
    readonly property color iconTextSelected: root.greenColor

    readonly property color buttonBackgroundColor: root.darkMode ? root.almostWhiteColor
                                                                 : root.greenColor
    readonly property color buttonTextColor: root.darkGrayColor
    readonly property color buttonTextPressedColor: root.darkMode ? root.greenColor
                                                                  : root.whiteColor
    readonly property color buttonRedTextPressedColor: root.redColor

    readonly property color backgroundColor: root.darkMode ? "#06054b" : root.whiteColor
    readonly property color darkBackgroundColor: root.darkMode ? "#020233" : "#f8fffc"

    readonly property color textGrayColor: root.grayColor
    readonly property color textMainColor: root.darkMode ? root.whiteColor : root.darkGrayColor
    readonly property color textSecondaryColor: root.darkMode ? root.lightGrayColor
                                                              : root.darkGrayColor
    readonly property color textGreenColor: root.greenColor
    readonly property color textRedColor: root.redColor

    readonly property color settingsSeparatorColor: root.darkMode ? "#21205d" : "#ebeaea"
    readonly property color settingsEntryBackground: root.darkMode ? "#080545" : "#f7f6f6"

    readonly property color legendBackgroundColor: root.darkMode ? "#010c6b" : "#d9feec"

    readonly property color backgroundBlurColor: root.darkMode ? Qt.rgba(0, 0, 0, 0.35)
                                                               : Qt.rgba(0, 0, 0, 0.1)

    readonly property color satelliteItemMainColor: root.darkMode ? root.almostWhiteColor
                                                                  : root.greenColor
    readonly property color satelliteItemSecondaryColor: root.darkMode ? root.grayColor
                                                                       : root.darkGrayColor
    readonly property color satelliteItemBorderColor: root.darkMode ? root.almostWhiteColor
                                                                    : "#8e8e8e"

    readonly property color separatorColor: Qt.rgba(root.grayColor.r,
                                                    root.grayColor.g,
                                                    root.grayColor.b,
                                                    0.3)
    readonly property color tableSeparatorColor: root.darkMode
                                                 ? Qt.rgba(root.lightGrayColor.r,
                                                           root.lightGrayColor.g,
                                                           root.lightGrayColor.b,
                                                           0.2)
                                                 : Qt.rgba(root.darkGrayColor.r,
                                                           root.darkGrayColor.g,
                                                           root.darkGrayColor.b,
                                                           0.2)

    readonly property color searchBorderColor: root.darkMode ? root.lightGrayColor
                                                             : root.grayColor

    // font
    readonly property int largeFontSize: 18
    readonly property int mediumFontSize: 14
    readonly property int smallFontSize: 12
    readonly property int fontDefaultWeight: Font.DemiBold
    readonly property int fontLightWeight: Font.Normal

    // some sizes
    readonly property int defaultSpacing: 5
}
