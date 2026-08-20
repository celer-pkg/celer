// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    width: 460
    height: 460

    // This exactly same content should appear 4 times, in different ways.
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
    }

    SceneComponent {
        id: sceneRoot2
    }

    // View1, importScene with node id of local component
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

    // View2, importScene with node id of external component
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
        importScene: sceneRoot2
    }

    // View3, importScene with external component
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
        importScene: SceneComponent {
            id: sceneRoot3
        }
    }

    // View4, content inside
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
        Node {
            id: sceneRoot4
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
        }
    }
}
