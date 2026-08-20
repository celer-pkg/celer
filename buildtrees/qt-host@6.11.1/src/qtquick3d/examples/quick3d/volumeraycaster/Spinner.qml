// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick

Image {
    required property bool running

    id: root
    source: "images/circle.png"
    width: 30
    height: 30
    visible: running
    mirrorVertically: true

    RotationAnimation on rotation {
        loops: Animation.Infinite
        from: 0
        to: 360
        duration: 700
        easing.type: Easing.OutInQuad
        running: root.running
    }
}
