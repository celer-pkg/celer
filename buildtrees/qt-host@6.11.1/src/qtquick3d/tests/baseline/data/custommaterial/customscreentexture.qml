// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

// This is like customdiffuse but a sphere in front that reads SCREEN_TEXTURE
// with the cylinder and the ground rectangle, and changes red to 1.0. Also
// keeps the alpha from the texture, so blending is exercised as well.

Rectangle {
    width: 400
    height: 400
    color: "#444845"

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.2, 0.2, 0.2, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        PointLight {
            position: Qt.vector3d(0, 500, 0)
            color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
            ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
            brightness: 5
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
        }

        Model {
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(5, 5, 5)
            eulerRotation.x: -90
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Shaded
                    vertexShader: "customdiffusespecular.vert"
                    fragmentShader: "customdiffuse.frag"
                    property real uTime: 0.0
                    property real uAmplitude: 0.0
                    property color uDiffuse: "white"
                    property real uShininess: 50
                }
            ]
        }

        Model {
            position: Qt.vector3d(-50, 0, -50)
            eulerRotation.x: 30.0
            eulerRotation.y: 100.0
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            source: "#Cylinder"
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Shaded
                    vertexShader: "customdiffusespecular.vert"
                    fragmentShader: "customdiffuse.frag"
                    property real uTime: 1.0
                    property real uAmplitude: 50.0
                    property color uDiffuse: "yellow"
                    property real uShininess: 50
                }
            ]
        }

        Model {
            id: screenSphere
            source: "#Sphere"
            scale: Qt.vector3d(4, 4, 4)
            z: 100
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Shaded
                    sourceBlend: CustomMaterial.SrcAlpha
                    destinationBlend: CustomMaterial.OneMinusSrcAlpha
                    fragmentShader: "customscreentexture.frag"
                }
            ]
        }
    }
}
