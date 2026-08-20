// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma ComponentBehavior: Bound

import QtQuick 6.5
import "components"
import content
import StocQt

Rectangle {
    id: root
    required property StateGroup stateGroup
    property var chosenElement: null
    color: "#101010"

    Rectangle {
        id: banner
        width: parent.width
        height: root.width < root.height? 70 : 0
        color: parent.color
        anchors.top: parent.top
        anchors.topMargin: 10
        visible: root.width < root.height
        Image {
            id: logoBig
            visible: root.width < root.height
            source: "images/qtLogo.png"
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
        }

        Image {
            id: logoSmall
            visible: root.width > root.height
            source: "images/qtLogo2.png"
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
        }
    }

    Search {
        id: searchBar
        anchors.top: banner.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    StockTitle {
        id: title
        width: parent.width
        height: 29
        anchors.top: searchBar.bottom
        date: qsTr("%1").arg(new Date().toLocaleDateString())
    }

    ListView {
        id: listView
        width: parent.width
        anchors.top: title.bottom
        anchors.bottom: parent.bottom
        clip: true
        boundsBehavior: Flickable.DragAndOvershootBounds
        model: StockEngine.filterModel

        currentIndex: -1
        delegate: StockDelegate {
            selectButton.onClicked: root.stateGroup.state = "StockView"
            width: listView.width
        }
    }
}
