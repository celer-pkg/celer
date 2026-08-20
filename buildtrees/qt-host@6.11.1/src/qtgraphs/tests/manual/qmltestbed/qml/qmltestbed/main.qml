// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Window {
    id: rootWindow

    readonly property real px: Math.max(width, height) / 1280

    QtObject {
        id: settings
        readonly property url startupView: "StartupView.qml"
        readonly property real iconSize: 30 + 30 * px
        readonly property real fontSizeSmall: 12 * px
        readonly property real fontSizeLarge: 26 * px
        property bool showSettingsView: true
    }

    width: 1280
    height: 720
    visible: true
    title: qsTr("Qt Quick Graphs Testbed")
    color: "#000000"

    Loader {
        id: loader
        anchors.fill: parent
        Component.onCompleted: setSource(settings.startupView, {loader: loader})
    }

    Button {
        id: backButton
        anchors.left: parent.left
        anchors.top: parent.top
        implicitWidth: settings.iconSize
        implicitHeight: settings.iconSize
        opacity: loader.source != "" && loader.source != settings.startupView
        visible: opacity
        icon.source: "qrc:/qml/qmltestbed/images/arrow_icon.png"
        icon.width: backButton.width * 0.3
        icon.height: backButton.height * 0.3
        icon.color: "transparent"
        background: Rectangle {
            color: "transparent"
        }
        onClicked: {
            loader.setSource(settings.startupView, {loader: loader})
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 400
                easing.type: Easing.InOutQuad
            }
        }
    }

}
