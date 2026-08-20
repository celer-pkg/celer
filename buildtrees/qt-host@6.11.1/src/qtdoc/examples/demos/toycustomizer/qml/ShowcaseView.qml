// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick3D
import QtQuick3D.Effects
import QtQuick3D.Helpers

View3D {
    id: view3D

    property alias isAnimationRunning: toyAnimations.isAnimationRunning

    environment: sceneEnvironment

    PrincipledMaterial {
        id: defaultMaterial
        roughness: 1
        alphaMode: PrincipledMaterial.Opaque
        objectName: "Default Material"
        baseColor: "#add6ff"
    }

    Texture {
        id: studio_small_01_4k
        source: "images/studio_small_01_4k.hdr"
        objectName: "Studio small 01 4k"
    }

    Node {
        id: scene

        PerspectiveCamera {
            id: sceneCamera
            y: 105
            z: 160
            clipFar: 2000
            clipNear: 1
            frustumCullingEnabled: false
            fieldOfView: 65
            eulerRotation.x: -5
        }

        ToyAnimations {
            id: toyAnimations
        }

        SpotLight {
            x: 80
            y: 220
            z: 280
            visible: true
            eulerRotation.x: -25
            color: "#e7e7e7"
            brightness: 16
            coneAngle: 100
            innerConeAngle: 26
            linearFade: 1.4
            quadraticFade: 0.3
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
            shadowBias: 5
            shadowFactor: 93
        }
    }

    SceneEnvironment {
        id: sceneEnvironment
        backgroundMode: SceneEnvironment.Transparent
        clearColor: "transparent"
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High

    }
}
