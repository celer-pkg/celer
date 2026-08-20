// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import QtQml

Item {
    id: mainView
    width: 1080
    height: 720

    TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        contentHeight: 30

        TabButton {
            text: qsTr("Basic Injection")
        }

        TabButton {
            text: qsTr("Node Injection")
        }

        TabButton {
            text: qsTr("Multi-view")
        }

        TabButton {
            text: qsTr("Multi-graph")
        }
    }

    StackLayout {
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: tabBar.currentIndex

        GraphInjection {}

        GraphNodeInjection {}

        MultiView {}

        MultiGraph {}


    }
}
