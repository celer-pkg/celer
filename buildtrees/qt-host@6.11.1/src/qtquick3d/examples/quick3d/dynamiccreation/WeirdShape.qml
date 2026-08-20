// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: weirdShape

    property real xRotation: Math.random() * (360 - (-360)) + -360
    property real yRotation: Math.random() * (360 - (-360)) + -360
    property real zRotation: Math.random() * (360 - (-360)) + -360

    property real hue: Math.random()

    Model {
        source: "weirdShape.mesh"
        scale: Qt.vector3d(150, 150, 150)
        eulerRotation.x: 90

        SequentialAnimation on eulerRotation {
            loops: Animation.Infinite
            PropertyAnimation {
                duration: Math.random() * (10000 - 1000) + 1000
                to: Qt.vector3d(weirdShape.xRotation -  360, weirdShape.yRotation - 360, weirdShape.zRotation - 360)
                from: Qt.vector3d(weirdShape.xRotation, weirdShape.yRotation, weirdShape.zRotation)
            }
        }

        materials: [
            PrincipledMaterial {
                baseColor: Qt.hsva(weirdShape.hue, 1.0, 1.0, 1.0)
                metalness: 0
                roughness: 0.1
            }
        ]
    }
}
