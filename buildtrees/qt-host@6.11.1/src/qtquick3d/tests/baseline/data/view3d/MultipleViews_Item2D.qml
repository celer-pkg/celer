// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    width: 460
    height: 460

    // 4 different views show the same sceneRoot
    // with different cameras.
    Node {
        id: sceneRoot
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 200)
        }
        DirectionalLight {

        }
        Model {
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseColor: "green"
            }
            eulerRotation: Qt.vector3d(45, 45, 45)
        }
        Node {
            eulerRotation.x: 60
            Rectangle {
                color: "red"
                width: 100
                height: 100
            }
        }
    }

    // View1, importScene with a camera in the sceneRoot
    View3D {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        width: 200
        height: 200
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: Qt.rgba(0.5, 0.5, 0.5, 1)
        }
        importScene: sceneRoot
    }

    // View2, importScene with a local perspective camea
    View3D {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        width: 200
        height: 200
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: Qt.rgba(0.5, 0.5, 0.5, 1)
        }
        importScene: sceneRoot
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 300)
        }
    }

    // View3, importScene with a local orthographic camera
    View3D {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 20
        width: 200
        height: 200
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: Qt.rgba(0.5, 0.5, 0.5, 1)
        }
        importScene: sceneRoot
        OrthographicCamera {
            position: Qt.vector3d(0, 0, 200)
        }
    }

    // View4, importScene from top view
    View3D {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        width: 200
        height: 200
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: Qt.rgba(0.5, 0.5, 0.5, 1)
        }
        importScene: sceneRoot
        PerspectiveCamera {
            position: Qt.vector3d(0, 200, 0)
            eulerRotation.x: -90
        }
    }
}
