// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

//! [DistortedCube Right]
Model {
    source: "qrc:/meshes/distortedcube.mesh"

    PrincipledMaterial {
        id: frontTop_material
        baseColor: "red"
        cullMode: Material.NoCulling
        lighting: PrincipledMaterial.NoLighting
    }

    PrincipledMaterial {
        id: frontBottom_material
        baseColor: "green"
        cullMode: Material.NoCulling
        lighting: PrincipledMaterial.NoLighting
    }

    PrincipledMaterial {
        id: leftSide_material
        baseColor: "blue"
        cullMode: Material.NoCulling
        lighting: PrincipledMaterial.NoLighting
    }

    PrincipledMaterial {
        id: rightSide_material
        baseColor: "pink"
        cullMode: Material.NoCulling
        lighting: PrincipledMaterial.NoLighting
    }

    PrincipledMaterial {
        id: backSide_material
        baseColor: "orange"
        cullMode: Material.NoCulling
        lighting: PrincipledMaterial.NoLighting
    }

    PrincipledMaterial {
        id: bottomSide_material
        baseColor: "navy"
        cullMode: Material.NoCulling
        lighting: PrincipledMaterial.NoLighting
    }

    materials: [
        frontTop_material,
        frontBottom_material,
        leftSide_material,
        backSide_material,
        rightSide_material,
        bottomSide_material
    ]
}
//! [DistortedCube Right]
