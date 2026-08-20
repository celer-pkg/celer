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
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                source: "maps/OpenfootageNET_garage-1024.hdr"
            }
        }

        // This sphere should receive reflections from the probe
        // that is nearest to it.
        Model {
            position: Qt.vector3d(-350, -30, 400)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            receivesReflections: true
            materials: [ DefaultMaterial {
                    diffuseColor: "lightgreen"
                    specularRoughness: 0.1
                    specularAmount: 1.0
                }
            ]

            ReflectionProbe {
                position: Qt.vector3d(0, 0, 0)
                boxSize: Qt.vector3d(1600, 400, 400)
                quality: ReflectionProbe.High
                refreshMode: ReflectionProbe.EveryFrame
                parallaxCorrection: false
            }
        }

        // This sphere should receive reflections from the probe
        // that is nearest to it.
        Model {
            position: Qt.vector3d(350, -30, 400)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            receivesReflections: true
            materials: [ PrincipledMaterial {
                    metalness: 1.0
                    roughness: 0.1
                    specularAmount: 1.0
                    baseColorMap: Texture { source: "maps/metallic/basecolor.jpg" }
                    metalnessMap: Texture { source: "maps/metallic/metallic.jpg" }
                    roughnessMap: Texture { source: "maps/metallic/roughness.jpg" }
                }
            ]

            ReflectionProbe {
                position: Qt.vector3d(0, 0, 0)
                boxSize: Qt.vector3d(1600, 400, 400)
                quality: ReflectionProbe.High
                refreshMode: ReflectionProbe.EveryFrame
                parallaxCorrection: false
            }
        }
    }
}
