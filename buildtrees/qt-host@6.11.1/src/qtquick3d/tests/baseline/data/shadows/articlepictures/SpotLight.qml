// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: window
    width: 800
    height: 800

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 400, 600)
            eulerRotation.x: -30
            clipFar: 2000
        }

        SpotLight {
            id: light2
            color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
            position: Qt.vector3d(0, 300, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            shadowMapFar: 2000
            shadowMapQuality: Light.ShadowMapQualityHigh
            visible: true
            castsShadow: true 
            brightness: 12
            coneAngle: 110
            shadowBias: 20
        }

        //! [rectangle models]
        Model {
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(15, 15, 15)
            eulerRotation.x: -90
            materials: [
                PrincipledMaterial {
                    baseColor: Qt.rgba(0.8, 0.6, 0.4, 1.0)
                }
            ]
        }
        Model {
            source: "#Rectangle"
            z: -400
            scale: Qt.vector3d(15, 15, 15)
            materials: [
                 PrincipledMaterial {
                    baseColor: Qt.rgba(0.8, 0.8, 0.9, 1.0)
                }
            ]
        }
        //! [rectangle models]

        Node {
            // Spot Light Marker
            position: light2.position
            rotation: light2.rotation
            property real size: 0.1
            scale: Qt.vector3d(size, size, size)
            Model {
                source: "#Cone"
                castsShadows: false
                eulerRotation.x: 90
                materials: PrincipledMaterial {
                    baseColor: light2.color
                    lighting: PrincipledMaterial.NoLighting
                }
            }
        }

        Model {
            materials: [PrincipledMaterial {
                baseColor: Qt.rgba(0.9, 0.9, 0.9, 1.0)
            }]
            y: 100
            scale: Qt.vector3d(5000, 5000, 5000)
            source: "../../shared/models/qt_logo.mesh"
        }
    }
}
