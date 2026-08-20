// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
Model {
    id: ship
    eulerRotation.z: 90
    eulerRotation.x: 90
    scale: Qt.vector3d(15, 25, 15)
    source: "meshes/ship.mesh"

    PrincipledMaterial {
        id: hull_material
        baseColor: "#ff779ccc"
        roughness: 0.15
        cullMode: Material.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    materials: [
        hull_material
    ]
}
