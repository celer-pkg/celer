// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    width: 480
    height: 480

    // Rectangles below and above View3D items
    Rectangle {
        z: 1
        width: 200
        height: 200
        anchors.centerIn: parent
        color: "#808080"
    }
    Rectangle {
        z: 10
        width: 150
        height: 150
        anchors.centerIn: parent
        color: "#606060"
    }

    Node {
        id: sceneRoot
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 350)
        }
        DirectionalLight {
        }
        Model {
            source: "#Cube"
            scale: Qt.vector3d(2,2,2)
            materials: DefaultMaterial {
                diffuseColor: "green"
            }
            eulerRotation: Qt.vector3d(45, 45, 45)
        }
    }

    View3D {
        z: 2
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        width: 200
        height: 200
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "red"
        }
        renderMode: View3D.Offscreen
        importScene: sceneRoot
        Node {
            z: 220
            Text {
                font.pixelSize: 22
                text: "Offscreen"
            }
        }
    }
    View3D {
        z: 3
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        width: 200
        height: 200
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
        }
        renderMode: View3D.Inline
        importScene: sceneRoot
        Node {
            z: 220
            Text {
                font.pixelSize: 22
                text: "Inline"
            }
        }
    }
    View3D {
        z: 4
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 20
        width: 200
        height: 200
        // Must match the window's default clear color (white). Only here for
        // Qt 5 compatibility. Not effective in Qt 6!
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "white"
        }
        renderMode: View3D.Underlay
        importScene: sceneRoot
        Node {
            z: 220
            Text {
                font.pixelSize: 22
                text: "Underlay"
            }
        }
    }
    View3D {
        z: 5
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        width: 200
        height: 200
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
        }
        renderMode: View3D.Overlay
        importScene: sceneRoot
        Node {
            z: 220
            Text {
                font.pixelSize: 22
                text: "Overlay"
            }
        }
    }
}
