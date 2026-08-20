// Copyright (C) 2022 The Qt Company Ltd.
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

    InnerShadow {
        anchors.fill: butterfly
        radius: 8.0
        samples: 16
        horizontalOffset: -3
        verticalOffset: 3
        color: "#b0000000"
        source: butterfly
    }
}
//! [example]
