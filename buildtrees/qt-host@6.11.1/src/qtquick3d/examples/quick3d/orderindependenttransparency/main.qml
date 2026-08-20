// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Window {
    id: rootWindow

    readonly property url startupView: "StartupView.qml"

    width: 1280
    height: 720
    visible: true
    title: qsTr("Qt Quick 3D Order Independent Transparency Testbed")
    color: "#000000"

    Component.onCompleted: {
        AppSettings.iconSize = Qt.binding(function() {
            return 16 + Math.max(rootWindow.width, rootWindow.height) * 0.05
        })
    }

    Loader {
        id: loader
        anchors.fill: parent
        Component.onCompleted: setSource(rootWindow.startupView, {loader: loader})
    }

    Button {
        id: backButton
        anchors.left: parent.left
        anchors.top: parent.top
        implicitWidth: AppSettings.iconSize
        implicitHeight: AppSettings.iconSize
        //height: width
        opacity: loader.source !== rootWindow.startupView
        visible: opacity
        icon.source: "qrc:/images/arrow_icon.png"
        icon.width: backButton.width * 0.3
        icon.height: backButton.height * 0.3
        icon.color: "transparent"
        background: Rectangle {
            color: "transparent"
        }
        onClicked: {
            loader.setSource(rootWindow.startupView, {loader: loader})
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 400
                easing.type: Easing.InOutQuad
            }
        }
    }

}
