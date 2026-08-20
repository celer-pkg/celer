// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    property alias usedInBakedLighting: cornellBox_Original.usedInBakedLighting
    property alias bakedLightmap: cornellBox_Original.bakedLightmap
    property alias texelsPerUnit: cornellBox_Original.texelsPerUnit

    // Materials
    PrincipledMaterial {
        id: floor_material
        baseColor: "#ffb9b5ad"
        roughness: 0.9
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: ceiling_material
        baseColor: "#ffb9b5ad"
        roughness: 0.9
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: backWall_material
        baseColor: "#ffb9b5ad"
        roughness: 0.9
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: rightWall_material
        baseColor: "#ff247317"
        roughness: 0.9
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: leftWall_material
        baseColor: "#ffa1110d"
        roughness: 0.9
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: shortBox_material
        baseColor: "#ffb9b5ad"
        roughness: 0.9
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: tallBox_material
        baseColor: "#ffb9b5ad"
        roughness: 0.9
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: light_material
        baseColor: "#ffc7c7c7"
        roughness: 0.9
        emissiveFactor: Qt.vector3d(1, 1, 1)
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: _material
        metalness: 1
        roughness: 1
        alphaMode: PrincipledMaterial.Opaque
    }
    // end of Materials

    Model {
        id: cornellBox_Original
        rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
        scale.y: 1
        scale.z: 1
        source: "box.mesh"
        materials: [
            floor_material,
            ceiling_material,
            backWall_material,
            rightWall_material,
            leftWall_material,
            shortBox_material,
            tallBox_material,
            light_material
        ]
    }
}
