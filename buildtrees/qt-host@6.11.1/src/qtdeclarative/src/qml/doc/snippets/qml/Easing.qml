// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
//! [0]
    Rectangle {
        id: rect
        width: 100
        height: 100
        anchors.centerIn: parent
        color: "red"
        opacity: 0
    }

    FrameAnimation {
        id: frameAnimation
        running: true

        property real elapsed // In seconds.
        readonly property real duration: 2 // Two seconds.

        onTriggered: {
            elapsed += frameTime
            // Loop once we reach the duration.
            if (elapsed > duration)
                elapsed = 0

            // Increase the opacity from 0 slowly at first, then quickly.
            rect.opacity = Easing.valueForProgress(Easing.InQuart, elapsed / duration)
        }
    }
//! [0]
}
