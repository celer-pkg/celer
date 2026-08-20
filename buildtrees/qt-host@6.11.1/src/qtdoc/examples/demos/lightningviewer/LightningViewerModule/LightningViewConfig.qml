// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQuick

QtObject {
    readonly property int appWidth: (preferredAppWidth > Screen.width)
                                    ? Screen.width : preferredAppWidth
    readonly property int appHeight: (preferredAppHeight > Screen.height)
                                     ? Screen.height : preferredAppHeight
    readonly property bool isPortrait: Screen.width < Screen.height
    readonly property bool isLandscape: !isPortrait
    readonly property int preferredAppWidth: Math.min(Screen.width, preferredScreenWidth)
    readonly property int preferredAppHeight: Math.min(Screen.height, preferredScreenHeight)
    readonly property int preferredScreenWidth: isLandscape ? 1280 : 720
    readonly property int preferredScreenHeight: isPortrait ? 1280 : 720
}
