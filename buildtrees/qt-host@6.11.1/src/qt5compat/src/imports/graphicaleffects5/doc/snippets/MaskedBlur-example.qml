// Copyright (C) 2022 The Qt Company Ltd.
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

    LinearGradient {
        id: mask
        anchors.fill: bug
        gradient: Gradient {
            GradientStop { position: 0.2; color: "#ffffffff" }
            GradientStop { position: 0.5; color: "#00ffffff" }
        }
        start: Qt.point(0, 0)
        end: Qt.point(300, 0)
        visible: false
    }

    MaskedBlur {
        anchors.fill: bug
        source: bug
        maskSource: mask
        radius: 16
        samples: 24
    }
}
//! [example]
