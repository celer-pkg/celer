// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics

CharacterController {
    collisionShapes:  CapsuleShape {
        height: 1
        diameter: 1
    }

    Model {
        eulerRotation.z: 90
        scale: Qt.vector3d(0.01, 0.01, 0.01)
        geometry: CapsuleGeometry {}
        materials: PrincipledMaterial {
            baseColor: "blue"
        }
    }
}
