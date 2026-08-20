// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs3D

Item {
    property alias value: valueAxis

    Value3DAxis {
        id: valueAxis
        min: 0
        max: 35

        SequentialAnimation on max {
            running: true
            loops: Animation.Infinite
            NumberAnimation {
                from: 20
                to: 80
                duration: 7000
                easing.type: Easing.InOutCubic
            }
            NumberAnimation {
                from: 80
                to: 20
                duration: 7000
                easing.type: Easing.InOutCubic
            }
        }
    }
}
