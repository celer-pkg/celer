// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: pointlight
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        anchors.fill: parent

        PerspectiveCamera {
            id: camera_u2473
            position: Qt.vector3d(-75, 75, -75)
            eulerRotation: Qt.vector3d(-35, -135, 0)
            clipFar: 5000
        }

        Texture {
            id: diffuseTexture
            source: "../shared/maps/texture_withAlpha.png"
        }

        Node {
            id: group

            Model {
                id: cube
                property real rotationy: 0.0
                position: Qt.vector3d(-19.6299, 18.21, -9.59015)
                scale: Qt.vector3d(0.25, 0.25, 0.25)
                eulerRotation: Qt.vector3d(0, rotationy, 0)
                source: "#Cube"

                materials: DefaultMaterial {
                    diffuseMap: diffuseTexture
                    depthDrawMode: Material.OpaquePrePassDepthDraw
                    cullMode: Material.NoCulling
                }
            }

            Model {
                id: car_u46272
                x: 35
                eulerRotation: Qt.vector3d(0, -180, 0)
                scale: Qt.vector3d(10, 10, 10)
                source: "../shared/models/carCombined.mesh"
                materials: DefaultMaterial {
                    diffuseMap: diffuseTexture
                    depthDrawMode: Material.OpaquePrePassDepthDraw
                }
            }

            Model {
                id: ground_u62818
                eulerRotation: Qt.vector3d(-90, 0, 0)
                scale: Qt.vector3d(7, 7, 1)
                source: "#Rectangle"
                castsShadows: false
                receivesShadows: true
                materials: DefaultMaterial {
                    diffuseColor: "#ff717171"
                }
            }
        }

        PointLight {
            id: light_u23253
            x: -23.3305
            y: 11.4227
            z: -8.09776
            color: "#ff38ffa6"
            ambientColor: "#ff191919"
            brightness: 1
            quadraticFade: 0
            castsShadow: true
            shadowBias: -0.1
            shadowFactor: 50
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
            shadowMapFar: 200
            shadowFilter: 0
            softShadowQuality: Light.Hard
        }
    }
}
