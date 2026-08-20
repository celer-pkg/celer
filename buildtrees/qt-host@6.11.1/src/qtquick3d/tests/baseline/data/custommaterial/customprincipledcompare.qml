// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Item {
    width: 400
    height: 400

    View3D {
        id: v3d
        anchors.fill: parent
        renderMode: View3D.Overlay

        environment: SceneEnvironment {
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
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

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            y: 200
            x: -100
            materials: PrincipledMaterial {
                baseColor: "#41cd52"
                metalness: 0.0
                roughness: 0.2
                specularAmount: 0.5
            }
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            y: 200
            x: 100
            materials: CustomMaterial {
                property color uBaseColor: "#41cd52"
                property real uMetalness: 0.0
                property real uRoughness: 0.2
                property real uSpecularAmount: 0.5
                fragmentShader: "customprincipledcompare.frag"
            }
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            x: -100
            materials: PrincipledMaterial {
                baseColor: "#41cd52"
                metalness: 0.3
                roughness: 0.2
                specularAmount: 0.0
            }
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            x: 100
            materials: CustomMaterial {
                property color uBaseColor: "#41cd52"
                property real uMetalness: 0.3
                property real uRoughness: 0.2
                property real uSpecularAmount: 0.0
                fragmentShader: "customprincipledcompare.frag"
            }
        }

        // Here the color is passed in as a non-color property (would be the
        // same if the shader hardcoded the value), so now it is up to the
        // shader to linearize.
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            y: -200
            materials: CustomMaterial {
                property vector4d uBaseColorSrgb: Qt.vector4d(0.2549, 0.80392, 0.32157, 1.0)
                property real uMetalness: 0.3
                property real uRoughness: 0.2
                property real uSpecularAmount: 0.0
                fragmentShader: "customprincipledcompare2.frag"
            }
        }
    }
}
