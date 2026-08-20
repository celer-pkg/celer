// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    required property var z_positions

    PrincipledMaterial {
        id: material
        baseColor: "gray"
    }

    Model {
        source: "#Cone"
        position: Qt.vector3d(0, 450, z_positions[0])
        eulerRotation.z: 180
        scale.y: 5
        materials: material
    }

    Model {
        source: "#Cone"
        position.z: z_positions[1]
        scale.y: 2.5
        materials: material
    }

    Model {
        source: "#Cylinder"
        position: Qt.vector3d(0, 175, z_positions[2])
        materials: material
        scale.y: 3.5
    }
}
