// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [example]
import QtQuick
//! [import]
import Qt5Compat.GraphicalEffects
//! [import]

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

    Image {
        id: butterfly
        source: "images/butterfly.png"
        sourceSize: Qt.size(parent.width, parent.height)
        smooth: true
        visible: false
    }

    Blend {
        anchors.fill: bug
        source: bug
        foregroundSource: butterfly
        mode: "subtract"
    }
}
//! [example]

