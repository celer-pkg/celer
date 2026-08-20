// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
        }

        camera: camera

        InstanceList {
            id: manualInstancing
            instances: [
                InstanceListEntry {
                    position: Qt.vector3d(0, 0, 0)
                    scale: Qt.vector3d(0.5, 0.5, 0.5)
                },
                InstanceListEntry {
                    position: Qt.vector3d(50, 200, 50)
                    scale: Qt.vector3d(0.5, 0.5, 0.5)
                },
                InstanceListEntry {
                    position: Qt.vector3d(-50, -200, -50)
                    scale: Qt.vector3d(0.5, 0.5, 0.5)
                }

            ]
        }

        PerspectiveCamera {
            id: camera
            z: 600
        }

        DirectionalLight {
            color: Qt.rgba(0.2, 0.2, 0.2, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        // morphTargets is defined but not used
        Model {
            source: "../shared/models/suzanne.mesh"
            instancing: manualInstancing
            scale: Qt.vector3d(80, 80, 80)
            x: -150
            materials: [
                CustomMaterial {
                    property vector4d offset: Qt.vector4d(100, 200, 300, 400)
                    vertexShader: "custommorph.vert"
                }
            ]
            morphTargets: [
                MorphTarget {
                    weight: 1
                    attributes: MorphTarget.Position | MorphTarget.Normal
                },
                MorphTarget {
                    weight: 1
                    attributes: MorphTarget.Position | MorphTarget.Normal
                }
            ]

        }

        Model {
            source: "../shared/models/suzanne.mesh"
            instancing: manualInstancing
            scale: Qt.vector3d(80, 80, 80)
            materials: [
                CustomMaterial {
                    property vector4d offset: Qt.vector4d(100, 200, 300, 400)
                    vertexShader: "custommorph.vert"
                }
            ]
        }

        Model {
            source: "../shared/models/suzanne.mesh"
            instancing: manualInstancing
            scale: Qt.vector3d(80, 80, 80)
            x: 150
            materials: [
                CustomMaterial {
                    property vector4d offset: Qt.vector4d(400, 100, 200, 300)
                    vertexShader: "custommorph.vert"
                }
            ]
        }
    }
}
