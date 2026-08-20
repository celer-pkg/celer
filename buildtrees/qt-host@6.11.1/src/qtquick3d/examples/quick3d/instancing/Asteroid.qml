// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
Model {
    id: asteroid
    source: "meshes/asteroid.mesh"

    PrincipledMaterial {
        id: asteroid_material
        baseColor: "#ffe0cc"
        roughness: 0.5
        cullMode: Material.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    materials: [
        asteroid_material
    ]
}
