// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//! [example]
import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    width: 300
    height: 300

    Image {
        id: background
        anchors.fill: parent
        source: "images/checker.png"
        smooth: true
        fillMode: Image.Tile
    }

    Image {
        id: bug
        source: "images/bug.jpg"
        sourceSize: Qt.size(parent.width, parent.height)
        smooth: true
        visible: false
    }

    Image {
        id: mask
        source: "images/fog.png"
        sourceSize: Qt.size(parent.width, parent.height)
        smooth: true
        visible: false
    }

    ThresholdMask {
        anchors.fill: bug
        source: bug
        maskSource: mask
        threshold: 0.4
        spread: 0.2
    }
}
//! [example]
