// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick3D.Helpers
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: "black"

    View3D {
        anchors.fill: parent
        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
        }

        DirectionalLight {
            id: light
            brightness: 1.5
            ambientColor: "#333"
        }

        RandomInstancing {
            id: randomInst
            randomSeed: 2021
            instanceCount: 200
            scale: InstanceRange {
                from: Qt.vector3d(0.1, 0.1, 0.1)
                to: Qt.vector3d(1, 1, 1)
                proportional: true
            }
            color: InstanceRange {
                from: "black"
                to: "white"
            }
            position: InstanceRange {
                from: Qt.vector3d(-200, -200, -100)
                to: Qt.vector3d(200, 200, 100)
            }
            NumberAnimation on instanceCountOverride {
                from: 10
                to: 100
                duration: 250
            }

        }

        Model {
            position: Qt.vector3d(0, 0, 0)
            instancing: randomInst
            source: "#Sphere"
            materials: [

                PrincipledMaterial {
                    metalness: 1.0
                    roughness: 0.7
                    baseColor: "white"
                }

            ]
        }
    }
}
