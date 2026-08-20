// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Model {
    id: lazer
    source: "lazer.mesh"
    scale: Qt.vector3d(100, 100, 100)
    property bool enableBeam: true

    PrincipledMaterial {
        id: material_001_material
        baseColorMap: Texture {
            source: "Lazer_baseColor.png"
            generateMipmaps: true
            mipFilter: Texture.Linear
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "Lazer_metalness.png"
            generateMipmaps: true
            mipFilter: Texture.Linear
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "Lazer_metalness.png"
            generateMipmaps: true
            mipFilter: Texture.Linear
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 0.304545
        alphaMode: PrincipledMaterial.Opaque
        lighting: PrincipledMaterial.NoLighting
    }
    materials: [
        material_001_material
    ]

    Beam {
        visible: lazer.enableBeam
    }
}
