// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    width: 900
    height: 450


    Node {
        id: scene

        readonly property int leftViewTag : ContentLayer.Layer1
        readonly property int rightViewTag : ContentLayer.Layer2

        Node {
            Model {
                layers: scene.leftViewTag
                id: leftModel
                position: Qt.vector3d(-50, 0, 0)
                source: "#Sphere"
                materials: [ DefaultMaterial {
                        diffuseColor: Qt.rgba(1, 0, 0, 1)
                        specularRoughness: 0.1
                        specularAmount: 1.0
                    }
                ]
            }
        }

        Node {
            Model {
                layers: scene.rightViewTag
                id: rightModel
                position: Qt.vector3d(50, 0, 0)
                source: "#Sphere"
                materials: [ PrincipledMaterial {
                        baseColor: Qt.rgba(0, 1, 0, 1)
                        metalness: 1.0
                        roughness: 0.1
                    }
                ]
            }
        }
    }

    // Left view should only show the left model
    View3D {
        id: viewLeft
        anchors.left: parent.left
        height: parent.height
        width: parent.width / 3

        Text {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Left Layer"
        }

        camera: PerspectiveCamera {
            position: Qt.vector3d(0, 0, 400)
            layers: scene.leftViewTag
        }

        DirectionalLight {
            layers: scene.leftViewTag

        }

        importScene: scene
    }

    // Center view should show both models, as no layers are specified so all models
    // are in the default layer.
    View3D {
        id: centerView
        anchors.centerIn: parent
        height: parent.height
        width: parent.width / 3

        Text {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Center Layer"
        }

        camera: PerspectiveCamera {
            position: Qt.vector3d(0, 0, 400)
        }

        DirectionalLight {

        }

        importScene: scene
    }

    // Right view should only show the right model
    View3D {
        id: viewRight
        anchors.right: parent.right
        height: parent.height
        width: parent.width / 3

        Text {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Right Layer"
        }

        // We intentionally don't set the camera explicitly here, to make sure
        // that the filtering works even if the camera automatically picked up.
        // NOTE: This is not recommended and will issue a warning in the console,
        // but it is common enought that we want to test it.
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 400)
            layers: scene.rightViewTag
        }

        DirectionalLight {
            layers: scene.rightViewTag
        }

        importScene: scene
    }
}
