// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick.Controls

Rectangle {
    id: window
    width: 640
    height: 360
    visible: true
    color: "green"

    Node {
        id: standAloneScene

        DirectionalLight {
            brightness: 1.0
            eulerRotation.x: -25
        }

        Model {
            source: "#Cube"
            y: 0
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation : Qt.vector3d(45,45,45)
            materials: [
                DefaultMaterial {
                    diffuseColor: "#FF00FF"
                }
            ]
        }

        OrthographicCamera {
            id: cameraOrthographic
            z: 600
            eulerRotation: Qt.vector3d(0, 0, 0)
        }
    }

    Rectangle {
        id: topLeft
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height
        color: window.color
        border.color: "black"

        View3D {
            id: topLeftView
            anchors.fill: parent
            importScene: standAloneScene
            camera: cameraOrthographic
            environment: SceneEnvironment
            {
                clearColor: window.color
                antialiasingMode: SceneEnvironment.MSAA
                antialiasingQuality: SceneEnvironment.VeryHigh
            }
        }

        Label {
            text: "MSAA VeryHigh"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 10
            color: "#222840"
            font.pointSize: 14
        }
    }

    Rectangle {
        id: topRight
        anchors.top: parent.top
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height
        color: window.color
        border.color: "black"

        View3D {
            id: topRightView
            anchors.fill: parent
            importScene: standAloneScene
            camera: cameraOrthographic
            environment: SceneEnvironment
            {
                clearColor: window.color
                antialiasingMode: SceneEnvironment.SSAA
                antialiasingQuality: SceneEnvironment.VeryHigh
            }
        }

        Label {
            text: "SSAA VeryHigh"
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            color: "#222840"
            font.pointSize: 14
        }
    }
}
