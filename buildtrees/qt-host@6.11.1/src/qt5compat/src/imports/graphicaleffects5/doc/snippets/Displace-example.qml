// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [example]
import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    width: 300
    height: 300

    Image {
        id: bug
        source: "images/bug.jpg"
        sourceSize: Qt.size(parent.width, parent.height)
        smooth: true
        visible: false
    }

    Rectangle {
        id: displacement
        color: Qt.rgba(0.5, 0.5, 1.0, 1.0)
        anchors.fill: parent
        visible: false
        Image {
            anchors.centerIn: parent
            source: "images/glass_normal.png"
            sourceSize: Qt.size(parent.width/2, parent.height/2)
            smooth: true
        }
    }

    Displace {
        anchors.fill: bug
        source: bug
        displacementSource: displacement
        displacement: 0.1
    }
}
//! [example]
