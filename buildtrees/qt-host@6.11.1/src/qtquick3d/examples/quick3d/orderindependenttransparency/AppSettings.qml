// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQuick
import QtQuick3D

QtObject {
    id: settings
    objectName: "test"
    // Antialiasing mode & quality used in all examples.
    property int antialiasingMode: SceneEnvironment.MSAA
    property int antialiasingQuality: SceneEnvironment.High
    // Toggle default visibility of these views
    property bool showSettingsView: true
    property bool showLoggingView: false
    // Fonts in pointSizes
    // These are used mostly on examples in 3D side
    property real fontSizeLarge: 16
    // These are used mostly on settings
    property real fontSizeSmall: 10
    property real iconSize: 24
    function testFunction() {
        console.log("testFunction called")
    }

}

