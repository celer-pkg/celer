// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 1024
    height: 600
    color: "black"

    View3D {
        anchors.fill: parent
        camera: camera

        DirectionalLight {
            eulerRotation.y: -100
            brightness: 1
        }

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                source: "../shared/maps/TestEnvironment-512.exr"
            }
            probeOrientation: Qt.vector3d(0, -90, 0)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        Model {
            position: Qt.vector3d(-250, -30, 0)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            materials: [ PrincipledMaterial {
                    baseColor: "#41cd52"
                    metalness: 0.5
                    roughness: 0.1
                    specularAmount: 0.35
                    specularTint: 0.2
                }
            ]
        }

        Model {
            position: Qt.vector3d(250, -30, 0)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            materials: [ PrincipledMaterial {
                    metalness: 0.5
                    roughness: 0.1
                    specularAmount: 0.35
                    specularTint: 0.2

                    baseColorMap: Texture { source: "maps/metallic/basecolor.jpg" }
                    metalnessMap: Texture { source: "maps/metallic/metallic.jpg" }
                    roughnessMap: Texture { source: "maps/metallic/roughness.jpg" }
                    normalMap: Texture { source: "maps/metallic/normal.jpg" }

                    metalnessChannel: Material.R
                    roughnessChannel: Material.R
                }
            ]
        }
    }
}
