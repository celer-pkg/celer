// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQml

QtObject {
    property int screenHeight: 480
    property int screenWidth: 320

    property int menuDelay: 500

    property int headerHeight: 20 // 70 on BB10
    property int footerHeight: 44 // 100 on BB10

    property int fontPixelSize: 14 // 55 on BB10

    property int blockSize: 32 // 64 on BB10

    property int toolButtonHeight: 32 // 64 on BB10

    property int menuButtonSpacing: 0 // 15 on BB10
}
