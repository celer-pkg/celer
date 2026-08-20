// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: root
    width: 640; height: 320
    color: "#646464"

    ListView {
        id: clockview
        anchors.fill: parent
        orientation: ListView.Horizontal
        cacheBuffer: 2000
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.ApplyRange

        delegate: Clock {
            required city
            required shift
        }
        model: ListModel {
            ListElement { city: "New York"; shift: -4 }
            ListElement { city: "London"; shift: 0 }
            ListElement { city: "Oslo"; shift: 1 }
            ListElement { city: "Mumbai"; shift: 5.5 }
            ListElement { city: "Tokyo"; shift: 9 }
            ListElement { city: "Brisbane"; shift: 10 }
            ListElement { city: "Los Angeles"; shift: -8 }
        }
    }

    Image {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 10
        source: "images/arrow.png"
        rotation: -90
        opacity: clockview.atXBeginning ? 0 : 0.5
        Behavior on opacity { NumberAnimation { duration: 500 } }
    }

    Image {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        source: "images/arrow.png"
        rotation: 90
        opacity: clockview.atXEnd ? 0 : 0.5
        Behavior on opacity { NumberAnimation { duration: 500 } }
    }
}
