// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: delegate

    required property real itemSize
    required property string name
    required property string feed
    required property string image
    required property int index
    required property bool isLoading

    property bool selected: ListView.isCurrentItem

    width: itemSize
    height: itemSize

    Image {
        anchors.centerIn: parent
        source: delegate.image
    }

    Text {
        id: titleText

        anchors {
            left: parent.left; leftMargin: 20
            right: parent.right; rightMargin: 20
            top: parent.top; topMargin: 20
        }

        font { pixelSize: 18; bold: true }
        text: delegate.name
        color: delegate.selected ? "#ffffff" : "#ebebdd"
        scale: delegate.selected ? 1.15 : 1.0
        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on scale { PropertyAnimation { duration: 300 } }
    }

    BusyIndicator {
        scale: 0.8
        visible: delegate.ListView.isCurrentItem && delegate.isLoading
        anchors.centerIn: parent
    }

    MouseArea {
        anchors.fill: delegate
        onClicked: {
            delegate.ListView.view.currentIndex = delegate.index
            delegate.clicked()
        }
    }
    signal clicked()
}
