// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: node
    required property url downloadBase

    // Nodes:
    Model {
        id: headlights_002
        objectName: "Headlights.002"
        y: 0.5664713978767395
        z: 1.7861577272415161
        source: node.downloadBase + "/asset_imports/Quick3DAssets/LightDecal/meshes/mesh_012_mesh.mesh"
        materials: [
            chrome_001_material
        ]
    }

    Node {
        id: __materialLibrary__

        PrincipledMaterial {
            id: chrome_001_material
            objectName: "Chrome.001"
            baseColor: "#393939"
            metalness: 1
            roughness: 0.0491
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
        }
    }
}
