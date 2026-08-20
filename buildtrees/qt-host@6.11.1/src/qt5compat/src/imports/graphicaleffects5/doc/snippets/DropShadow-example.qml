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
    }

    Image {
        id: butterfly
        source: "images/butterfly.png"
        sourceSize: Qt.size(parent.width, parent.height)
        smooth: true
        visible: false
    }

    DropShadow {
        anchors.fill: butterfly
        horizontalOffset: 3
        verticalOffset: 3
        radius: 8.0
        color: "#80000000"
        source: butterfly
    }
}
//! [example]
