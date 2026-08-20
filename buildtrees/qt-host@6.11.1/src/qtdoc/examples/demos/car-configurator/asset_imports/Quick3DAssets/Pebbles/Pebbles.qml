// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: node
    required property url downloadBase
    // Nodes:
    Model {
        id: plane
        objectName: "Plane"
        source: node.downloadBase + "/asset_imports/Quick3DAssets/Pebbles/meshes/plane_mesh.mesh"
        materials: [
            mat_material,
            grass_material
        ]
    }

    Node {
        id: __materialLibrary__

        PrincipledMaterial {
            id: mat_material
            baseColorMap: venodhb_2K_Albedo
            objectName: "mat"
            baseColor: "#ffffff"
            roughness: 0.78192
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
        }

        PrincipledMaterial {
            id: grass_material
            opacity: 0.91
            lighting: PrincipledMaterial.NoLighting
            blendMode: PrincipledMaterial.SourceOver
            objectName: "grass"
            baseColor: "#718457"
            roughness: 0.84172
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
        }

        Texture {
            id: venodhb_2K_Albedo
            source: node.downloadBase + "/content/images/venodhb_2K_Albedo.jpg"
        }
    }
}
