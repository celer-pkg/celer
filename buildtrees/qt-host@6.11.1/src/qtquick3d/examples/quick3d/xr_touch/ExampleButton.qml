// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    required property real degrees
    property bool on: false
    function hit(scenePos : vector3d) : bool {
        const localPos = mapPositionFromScene(scenePos)
        const result = Math.abs(localPos.x) < 5 && Math.abs(localPos.z) < 2.5 && localPos.y < 2 && localPos.y > 0
        return result
    }

    Model {
        scale: Qt.vector3d(0.10, 0.03, 0.05)
        y: on ? -1 : 1
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColor: "#111133"
        }
        Behavior on y { NumberAnimation {duration: 300} }
    }
}
