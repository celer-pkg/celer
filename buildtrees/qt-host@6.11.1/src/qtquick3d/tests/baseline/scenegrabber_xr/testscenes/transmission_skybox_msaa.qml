// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 0, 360)
    property SceneEnvironment qmlxr_environment: SceneEnvironment {
        clearColor: "white"
        backgroundMode: SceneEnvironment.SkyBox
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
        lightProbe: proceduralSky
    }
    Texture {
        id: proceduralSky
        textureData: ProceduralSkyTextureData {
            sunLongitude: -115
        }
    }
    Node {
        DirectionalLight {
            eulerRotation: Qt.vector3d(-45, 25, 0)
        }
        PrincipledMaterial {
            id: glassMaterial
            baseColor: "#aaaacc"
            transmissionFactor: 0.95
            thicknessFactor: 1
            roughness: 0.05
        }
        Model {
            source: "#Sphere"
            materials: glassMaterial
        }
    }
}
