// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 0, 100)
    property SceneEnvironment qmlxr_environment: ExtendedSceneEnvironment {
        backgroundMode: SceneEnvironment.SkyBoxCubeMap
        lightProbe: skyTexture
        skyBoxCubeMap: skyboxTexture

        vignetteEnabled: true

        glowEnabled: true
        glowBloom: 0.5
        glowBlendMode: ExtendedSceneEnvironment.GlowBlendMode.Additive
        glowLevel: ExtendedSceneEnvironment.GlowLevel.One | ExtendedSceneEnvironment.GlowLevel.Two | ExtendedSceneEnvironment.GlowLevel.Three
    }

    CubeMapTexture {
        id: skyboxTexture
        source: "maps/skybox/right.jpg;maps/skybox/left.jpg;maps/skybox/top.jpg;maps/skybox/bottom.jpg;maps/skybox/front.jpg;maps/skybox/back.jpg"
    }

    Texture {
        id: skyTexture
        textureData: ProceduralSkyTextureData {
            id: proceduralSkyTextureData
            groundBottomColor: "#775533"
            groundHorizonColor: "green"
            groundCurve: 0.11

            skyTopColor: "#ddeeff"
            skyHorizonColor: "#aaaaff"
            skyCurve: 0.15
        }
    }

    DirectionalLight {
    }

    Model {
        source: "#Sphere"
        materials: [ PrincipledMaterial {
                id: sphereMaterial
                baseColor: "green"
                metalness: 0.5
            } ]
    }
}
