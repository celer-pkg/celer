// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

// There are three monkey things here, with two spheres on top.
// The left sphere uses mip level 0 and just colorizes a bit.
// The right sphere samples a different mip level (SCREEN_MIP_TEXTURE).

Rectangle {
    width: 400
    height: 400
    color: "#444845"

    View3D {
        id: v3d
        anchors.fill: parent

//        environment: SceneEnvironment {
//            clearColor: "#444845"
//            backgroundMode: SceneEnvironment.Color
//        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
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
            source: "../shared/models/monkey_object.mesh"
            scale: Qt.vector3d(80, 80, 80)
            eulerRotation.y: 90
            x: -200
            materials: [
                CustomMaterial {
                    fragmentShader: "worldnormal.frag"
                    cullMode: CustomMaterial.NoCulling
                }
            ]
        }

        Model {
            source: "../shared/models/monkey_object.mesh"
            scale: Qt.vector3d(80, 80, 80)
            eulerRotation.y: 90
            materials: [
                CustomMaterial {
                    fragmentShader: "worldnormal.frag"
                    cullMode: CustomMaterial.NoCulling
                }
            ]
        }

        Model {
            source: "../shared/models/monkey_object.mesh"
            scale: Qt.vector3d(80, 80, 80)
            eulerRotation.y: 90
            x: 200
            materials: [
                CustomMaterial {
                    fragmentShader: "worldnormal.frag"
                    cullMode: CustomMaterial.NoCulling
                }
            ]
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(4, 4, 4)
            z: 100
            x: -200
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Shaded
                    sourceBlend: CustomMaterial.SrcAlpha
                    destinationBlend: CustomMaterial.OneMinusSrcAlpha
                    fragmentShader: "customscreenmiptexture.frag"
                    property real mipLevel: 0
                }
            ]
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(4, 4, 4)
            z: 100
            x: 200
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Shaded
                    sourceBlend: CustomMaterial.SrcAlpha
                    destinationBlend: CustomMaterial.OneMinusSrcAlpha
                    fragmentShader: "customscreenmiptexture.frag"
                    property real mipLevel: 3
                }
            ]
        }
    }
}
