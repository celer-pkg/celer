// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D

Node {
    id: firewood
    property alias instancing: model.instancing

    // Resources
    Texture {
        id: firewood_bc
        generateMipmaps: true
        source: "media/textures/firewood/firewood_bc.jpg"
    }
    Texture {
        id: firewood_r
        generateMipmaps: true
        source: "media/textures/firewood/firewood_r.jpg"
    }
    Texture {
        id: firewood_n
        generateMipmaps: true
        source: "media/textures/firewood/firewood_n.jpg"
    }
    Texture {
        id: firewood_a
        generateMipmaps: true
        source: "media/textures/firewood/firewood_a.jpg"
    }
    PrincipledMaterial {
        id: firewood_material
        baseColorMap: firewood_bc
        metalnessMap: firewood_r
        occlusionMap: firewood_a
        // metalness: 1
        baseColor: "gray"
        roughness: 1
        normalMap: firewood_n
    }

    Model {
        id: model
        rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
        scale: Qt.vector3d(100, 100, 100)
        source: "media/meshes/firewood/plane_mesh.mesh"
        materials: [
            firewood_material
        ]
    }

    // Animations:
}
