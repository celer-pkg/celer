// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick3D

Node {
    PrincipledMaterial {
        id: whiteMaterial
        baseColor: "white"
        roughness: 0.8
    }
    Texture {
        id: tileColor_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tiles107_1K_Color.jpg"
    }
    Texture {
        id: tileNormal_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tiles107_1K_NormalGL.jpg"
    }
    Texture {
        id: tileRoughness_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tiles107_1K_Roughness.jpg"
    }

    Texture {
        id: blackTileColor_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tiles108_1K_Color.jpg"
    }

    PrincipledMaterial {
        id: internalWallMaterial
        baseColorMap: tileColor_texture
        roughnessMap: tileRoughness_texture
        roughness: 1
        normalMap: tileNormal_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: testFloor
        baseColorMap: blackTileColor_texture
        roughnessMap: tileRoughness_texture
        roughness: 1
        normalMap: tileNormal_texture
        normalStrength: 0.5
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    Texture {
        id: warningColor_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tape001_1K_Color.jpg"
    }
    Texture {
        id: warningNormal_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tape001_1K_NormalGL.jpg"
    }
    Texture {
        id: warningRoughness_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tape001_1K_Roughness.jpg"
    }

    PrincipledMaterial {
        id: warningMaterial
        baseColorMap: warningColor_texture
        roughnessMap: warningRoughness_texture
        roughness: 1
        normalMap: warningNormal_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    Model {
        pickable: true
        source: "meshes/building.mesh"
        materials: [testFloor, internalWallMaterial, whiteMaterial, warningMaterial]
    }
}
