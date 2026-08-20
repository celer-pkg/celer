// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQuick

QtObject {
    property string currentView: "RoomsView"

    property bool isBigDesktopLayout: layout === Constants.Layout.Desktop
    property bool isSmallDesktopLayout: layout === Constants.Layout.SmallDesktop
    property bool isMobileLayout: layout === Constants.Layout.Mobile
    property bool isSmallLayout: layout === Constants.Layout.Small

    enum Layout { Desktop, SmallDesktop, Mobile, Small }
    property int layout: Constants.Layout.Desktop

    readonly property font smallTitleFont: Qt.font({
            "family": "Inter",
            "pixelSize": 14,
            "weight": 700
        })

    readonly property font mobileTitleFont: Qt.font({
            "family": "Titillium Web",
            "pixelSize": 24,
            "weight": 600
        })

    readonly property font desktopTitleFont: Qt.font({
            "family": "Titillium Web",
            "pixelSize": 48,
            "weight": 600
        })

    readonly property font desktopSubtitleFont: Qt.font({
            "family": "Titillium Web",
            "pixelSize": 24,
            "weight": 600
        })

    readonly property color backgroundColor: AppSettings.isDarkTheme ? "#000000" : "#EFFCF6"
    readonly property color accentColor: AppSettings.isDarkTheme ? "#002125" : "#FFFFFF"
    readonly property color primaryTextColor: AppSettings.isDarkTheme ? "#FFFFFF" : "#000000"
    readonly property color accentTextColor: AppSettings.isDarkTheme ? "#D9D9D9" : "#898989"
    readonly property color iconColor: AppSettings.isDarkTheme ? "#D9D9D9" : "#00414A"
}
