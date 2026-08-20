// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: pointer

    property real beamThickness: 0.3
    property color beamColor: "#eeeeee"
    property color beamHitColor: "#bbddcc"
    property color beamPressedHitColor: "#bbccdd"
    property color beamPressedMissColor: "#ddbbbb"
    property bool pressed: false
    property bool hit: false

    property real length: 150

    Model {
        readonly property real thickness: pointer.beamThickness / 100
        eulerRotation.x: -90
        scale: Qt.vector3d(thickness, pickRay.length/100, thickness)
        source: "#Cone"
        materials: PrincipledMaterial {
            baseColor: pointer.hit ?
                           (pointer.pressed ? pointer.beamPressedHitColor : pointer.beamHitColor) :
                           (pointer.pressed ? pointer.beamPressedMissColor : pointer.beamColor)
            lighting: PrincipledMaterial.NoLighting
        }
        opacity: 0.8
    }
}
