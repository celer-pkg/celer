// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 0, 1200)
    property SceneEnvironment qmlxr_environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.SkyBoxCubeMap
        skyBoxCubeMap: skyboxTexture
    }

    CubeMapTexture {
        id: skyboxTexture
        source: "maps/skybox/right.jpg;maps/skybox/left.jpg;maps/skybox/top.jpg;maps/skybox/bottom.jpg;maps/skybox/front.jpg;maps/skybox/back.jpg"
    }

    DirectionalLight { }

    ReflectionProbe {
        position: Qt.vector3d(0, 0, 0)
        boxSize: Qt.vector3d(3000, 3000, 3000)
        quality: ReflectionProbe.High
        refreshMode: ReflectionProbe.EveryFrame
        parallaxCorrection: true
    }

    Model {
        position: Qt.vector3d(-350, -30, 400)
        scale: Qt.vector3d(4, 4, 4)
        source: "#Sphere"
        receivesReflections: true
        materials: [ DefaultMaterial {
                diffuseColor: "green"
                specularRoughness: 0.1
                specularAmount: 1.0
            }
        ]
    }

    Model {
        position: Qt.vector3d(350, -30, 400)
        scale: Qt.vector3d(4, 4, 4)
        source: "#Sphere"
        receivesReflections: false // This sphere should not receive any reflections from the probe
        materials: [ PrincipledMaterial {
                metalness: 1.0
                roughness: 0.1
                specularAmount: 1.0
                baseColorMap: Texture { source: "maps/metallic/basecolor.jpg" }
                metalnessMap: Texture { source: "maps/metallic/metallic.jpg" }
                roughnessMap: Texture { source: "maps/metallic/roughness.jpg" }
            }
        ]
    }
}
