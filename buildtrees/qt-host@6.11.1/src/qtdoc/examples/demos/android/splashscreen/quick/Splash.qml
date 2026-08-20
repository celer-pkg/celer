// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick

//! [Splash view header]
Window {
    id: splash
    color: "#2CDE85"
    title: qsTr("Splash Window")
    modality: Qt.ApplicationModal
    flags: Qt.SplashScreen
    visible: true

    property int timeoutInterval: 2000
    signal timeout
//! [Splash view header]

//! [Loader]
    Loader {
        id: mainLoader
        source: "Main.qml"
//! [Loader]
    }

    Image {
        id: splashImage
        visible: true
        source: "qt_logo.png"
        fillMode: Image.PreserveAspectFit
        width: splash.width * 3/4
        anchors.centerIn: parent
    }

//! [Exit function for timer]
    function exit() {
        mainLoader.item.show();
        splash.visible = false
        splash.timeout()
    }
//! [Exit function for timer]
//! [The timer]
    Timer {
        interval: splash.timeoutInterval;
        running: splash.visible;
        repeat: false
        onTriggered: {
            splash.exit()
        }
    }
//! [The timer]
}
