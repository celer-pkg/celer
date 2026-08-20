// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 200, 300)
    property vector3d qmlxr_originRotation: Qt.vector3d(-20, 0, 0)

    property SceneEnvironment qmlxr_environment: SceneEnvironment {
        clearColor: "skyblue"
        backgroundMode: SceneEnvironment.Color

        effects: [ spiralEffect, redEffect ]

        //antialiasingMode: SceneEnvironment.SSAA
        //antialiasingQuality: SceneEnvironment.High
    }

    Effect {
        id: spiralEffect
        passes: Pass {
            shaders: [
                Shader {
                    stage: Shader.Vertex
                    shader: "spiral.vert"
                },
                Shader {
                    stage: Shader.Fragment
                    shader: "spiral.frag"
                }
            ]
        }
    }

    Effect {
        id: redEffect
        property real uRed: 1.0
        NumberAnimation on uRed { from: 1; to: 0; duration: 5000; loops: -1 }
        passes: Pass {
            shaders: Shader {
                stage: Shader.Fragment
                shader: "red.frag"
            }
        }
    }

    DirectionalLight {
        eulerRotation.x: -20
        eulerRotation.y: 20
        ambientColor: Qt.rgba(0.8, 0.8, 0.8, 1.0);
    }

    Texture {
        id: checkers
        source: "maps/checkers2.png"
        scaleU: 20
        scaleV: 20
        tilingModeHorizontal: Texture.Repeat
        tilingModeVertical: Texture.Repeat
    }

    Model {
        source: "#Rectangle"
        scale.x: 10
        scale.y: 10
        eulerRotation.x: -90
        materials: [ DefaultMaterial { diffuseMap: checkers } ]
    }

    Model {
        source: "#Cone"
        position: Qt.vector3d(100, 0, -200)
        scale.y: 3
        materials: [ DefaultMaterial { diffuseColor: "green" } ]
    }

    Model {
        id: sphere
        source: "#Sphere"
        position: Qt.vector3d(-100, 200, -200)
        materials: [ DefaultMaterial { diffuseColor: "#808000" } ]
    }

    Model {
        source: "#Cube"
        position.y: 50
        eulerRotation.y: 20
        materials: [ DefaultMaterial { diffuseColor: "gray" } ]
    }
}
