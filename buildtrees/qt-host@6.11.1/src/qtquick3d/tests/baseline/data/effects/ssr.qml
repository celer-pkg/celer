// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Item {
    width: 640
    height: 480

    Item {
        anchors.fill: parent

        View3D {
            id: v3d
            anchors.fill: parent

            environment: ExtendedSceneEnvironment {
                clearColor: "lightblue"
                backgroundMode: SceneEnvironment.Color
                ssrEnabled: true
            }

            Model {
                source: "#Rectangle"
                y: -200
                scale: Qt.vector3d(50, 50, 50)
                eulerRotation.x: -90
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                    roughness: 0.3
                    metalness: 0.2
                }
            }

            Model {
                source: "#Cube"

                scale: Qt.vector3d(2, 2, 2)
                x: 300
                z: 300
                y: -100
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.9, 0.4, 0.4, 1.0)
                }
            }


            Model {
                source: "#Cone"

                scale: Qt.vector3d(4, 4, 4)
                x: -300
                z: -300
                y: 50
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.1, 0.9, 0.4, 1.0)
                }
            }


            DirectionalLight {
                id: dirLight
                visible: true
                eulerRotation.x: -60
                castsShadow: true
                shadowFactor: 90
                brightness: 1
                shadowMapQuality: Light.ShadowMapQualityHigh
                softShadowQuality: Light.PCF8
                pcfFactor: 2
            }

            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 100, 1500)
            }

            WasdController {
                controlledObject: camera
            }
        }
    }
}
