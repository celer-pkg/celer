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
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: theFog.color
            fog: Fog {
                id: theFog
                enabled: true
                depthEnabled: true
                density: 0.7
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
            randomSeed: 2022
        }

        Model {
            instancing: randomInstancing
            source: "#Sphere"
            materials: CustomMaterial {
                fragmentShader: "fog_with_custommaterial.frag"
            }
            scale: Qt.vector3d(0.005, 0.005, 0.005)
        }
    }
}
