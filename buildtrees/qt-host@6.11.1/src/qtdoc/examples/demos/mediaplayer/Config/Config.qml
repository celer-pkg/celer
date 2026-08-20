// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQuick

QtObject {
    enum Theme {
        Light,
        Dark
    }

    property int activeTheme : Config.Theme.Dark

    readonly property bool isMobileTarget : Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property color mainColor : activeTheme ? "#09102B" : "#FFFFFF"
    readonly property color secondaryColor : activeTheme ? "#FFFFFF" : "#09102B"
    readonly property color highlightColor : "#41CD52"

    function iconName(fileName, addSuffix = true) {
        return `${fileName}${activeTheme === Config.Theme.Dark && addSuffix ? "_Dark.svg" : ".svg"}`
    }
}
