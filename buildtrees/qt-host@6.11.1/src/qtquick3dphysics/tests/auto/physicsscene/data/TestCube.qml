// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Physics

// TestBody is a DynamicRigidBody with a "stable" property

DynamicRigidBody {
    property alias color: cubemat.baseColor
    massMode: DynamicRigidBody.CustomDensity
    density: 1000
    collisionShapes: BoxShape { extents: Qt.vector3d(1, 1, 1) }
    Model {
        source: "#Cube"
        scale: Qt.vector3d(1, 1, 1).times(0.01)
        eulerRotation: Qt.vector3d(0, 90, 0)
        materials: PrincipledMaterial {
            id: cubemat
            baseColor: "red"
        }
    }
    property bool stable: false
    property vector3d prevPos: "9999,9999,9999"
    property quaternion prevRot
    function checkStable() {
        if (position === prevPos && rotation === prevRot) {
            stable = true
        } else {
            prevPos = position
            prevRot = rotation
        }
    }
}
