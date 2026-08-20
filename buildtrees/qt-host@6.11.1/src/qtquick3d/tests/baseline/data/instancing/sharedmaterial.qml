// Copyright (C) 2023 The Qt Company Ltd.
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
            position: Qt.vector3d(100, -100, 0)
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

        PrincipledMaterial {
            id: sharedMaterial
            baseColor: "white"
        }

        Model {
            id: instancedCube
            instancing: manualInstancing
            source: "#Cube"
            materials: sharedMaterial
        }

        Model {
            id: cube
            position: Qt.vector3d(-100, 100, 0)
            source: "#Cube"
            materials: sharedMaterial
        }
    }
}
