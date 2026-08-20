// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: mainWindow
    height: 480
    width: 320
    visible: true;

    Rectangle {
        id: mainRect
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#87E0FD"
            }
            GradientStop {
                position: 0.4
                color: "#53CBF1"
            }
            GradientStop {
                position: 1.0
                color: "#05ABE0"
            }
        }
        anchors.fill: parent

        Loader {
            id: gameLoader
            asynchronous: true
            visible: status == Loader.Ready
            anchors.fill: parent
        }

        Loader {
            id: splashLoader
            anchors.fill: parent
            source: "qml/SplashScreen.qml"
            onLoaded: gameLoader.source = "qml/MainView.qml";
        }
    }
}
