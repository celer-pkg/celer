// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    width: 900
    height: 450

    View3D {
        anchors.fill: parent
        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 1200)


        }

        environment: SceneEnvironment {
            id: sceneEnv
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }

        ReflectionProbe {
            position: Qt.vector3d(0, 0, 0)
            boxSize: Qt.vector3d(500, 500, 500)
            quality: ReflectionProbe.High
            refreshMode: ReflectionProbe.EveryFrame
            texture: CubeMapTexture {
                source: "../shared/maps/cubemap_bc1.ktx"
            }

            NumberAnimation on eulerRotation.y {
                running: true
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 10000
            }
        }

        Model {
            position: Qt.vector3d(-350, -30, 400)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            receivesReflections: true
            materials: [ DefaultMaterial {
                    diffuseColor: Qt.rgba(1, 1, 1, 1)
                    specularRoughness: 0.1
                    specularAmount: 1.0
                }
            ]
        }

        Model {
            position: Qt.vector3d(350, -30, 400)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            receivesReflections: true
            materials: [ PrincipledMaterial {
                    baseColor: Qt.rgba(1, 1, 1, 1)
                    metalness: 1.0
                    roughness: 0.1
                }
            ]
        }
    }
}
