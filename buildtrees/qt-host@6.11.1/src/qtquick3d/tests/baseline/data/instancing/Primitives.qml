// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: primitives
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0
        width: parent.width * 1
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        InstanceListEntry {
            id: redInstance
            position: Qt.vector3d(0, 0, 0)
            color: "red"
        }

        InstanceListEntry {
            id: greenInstance
            position: Qt.vector3d(100, 100, 0)
            eulerRotation: Qt.vector3d(-10, 0, 30)
            color: "green"
        }

        InstanceListEntry {
            id: blueInstance
            position: Qt.vector3d(-100, -100, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            color: "blue"
        }

        InstanceList {
            id: manualInstancing
            instances: [ redInstance, greenInstance, blueInstance ]
        }


        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
            clipFar: 5000
        }

        DirectionalLight {
            id: light
            shadowFactor: 10
        }

        Model {
            id: rectangle
            instancing: manualInstancing
            instanceRoot: rectangle
            position: Qt.vector3d(-409.906, 211.328, 0)
            source: "#Rectangle"



            DefaultMaterial {
                id: default_
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_]
        }

        Model {
            id: sphere
            instancing: manualInstancing
            instanceRoot: sphere
            position: Qt.vector3d(-404.139, 0, 0)
            source: "#Sphere"



            DefaultMaterial {
                id: default_001
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_001]
        }

        Model {
            id: cube
            instancing: manualInstancing
            instanceRoot: cube
            position: Qt.vector3d(8.66024, 170.318, 0)
            source: "#Cube"



            DefaultMaterial {
                id: default_002
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_002]
        }

        Model {
            id: cylinder
            instancing: manualInstancing
            instanceRoot: cylinder
            position: Qt.vector3d(0, -152.968, 0)
            source: "#Cylinder"



            DefaultMaterial {
                id: default_003
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_003]
        }

        Model {
            id: cone
            instancing: manualInstancing
            instanceRoot: cone
            position: Qt.vector3d(349.224, 0, 0)
            source: "#Cone"



            DefaultMaterial {
                id: default_004
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_004]
        }
    }
}
