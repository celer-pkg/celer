// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Rectangle {
    id: defaults
    width: 800
    height: 400
    color: Qt.rgba(1, 1, 1, 1)

    View3D {
        width: parent.width / 2
        height: parent.height
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: theFog.color
            fog: Fog {
                id: theFog
                enabled: true
                depthEnabled: true
            }
        }

        PerspectiveCamera {
            z: 600
        }

        DirectionalLight {
        }

        PointLight {
            y: 100
        }

        RandomInstancing {
            id: randomInstancing
            instanceCount: 2000

            position: InstanceRange {
                from: Qt.vector3d(-500, -300, 0)
                to: Qt.vector3d(500, 300, -2000)
            }
            scale: InstanceRange {
                from: Qt.vector3d(1, 1, 1)
                to: Qt.vector3d(50, 50, 50)
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(180, 180, 180)
            }
            color: InstanceRange {
                from: "#000000"
                to: "#ffffff"
            }
            randomSeed: 2022
        }

        Model {
            instancing: randomInstancing
            source: "#Sphere"
            materials: PrincipledMaterial { }
            scale: Qt.vector3d(0.005, 0.005, 0.005)
        }
    }

    View3D {
        x: parent.width / 2
        width: parent.width / 2
        height: parent.height
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: theOtherFog.color
            fog: Fog {
                id: theOtherFog
                enabled: true

                depthEnabled: true
                depthNear: 300.0
                depthCurve: 0.6

                color: "yellow"

                transmitEnabled: true
                transmitCurve: 0.5

                heightEnabled: true
                heightCurve: 0.7
            }
        }

        PerspectiveCamera {
            z: 300
        }

        DirectionalLight {
        }

        PointLight {
            y: 100
        }

        Model {
            instancing: randomInstancing
            source: "#Sphere"
            materials: PrincipledMaterial { }
            scale: Qt.vector3d(0.005, 0.005, 0.005)
        }
    }
}
