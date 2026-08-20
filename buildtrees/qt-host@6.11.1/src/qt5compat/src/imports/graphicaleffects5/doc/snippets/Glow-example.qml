// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [example]
import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    width: 300
    height: 300

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Image {
        id: butterfly
        source: "images/butterfly.png"
        sourceSize: Qt.size(parent.width, parent.height)
        smooth: true
        visible: false
    }

    Glow {
        anchors.fill: butterfly
        radius: 8
        color: "white"
        source: butterfly
    }
}
//! [example]
