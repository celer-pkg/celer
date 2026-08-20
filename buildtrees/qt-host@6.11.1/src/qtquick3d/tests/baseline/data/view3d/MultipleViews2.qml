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
        opacity: 0.8
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
        opacity: 0.8
    }

    // View1, importScene with node id of local component + content inside
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
        Component.onCompleted: {
            importScene = sceneRoot;
        }

        Model {
            // Behind, so partly visible
            source: "#Cube"
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            y: 50
            z: -120
            materials: DefaultMaterial {
                diffuseColor: "red"
            }
            eulerRotation: Qt.vector3d(45, 45, 45)
        }
        Model {
            // Above, so fully visible
            source: "#Cube"
            scale: Qt.vector3d(0.1, 0.1, 0.1)
            y: -10
            z: 120
            materials: DefaultMaterial {
                diffuseColor: "blue"
            }
            eulerRotation: Qt.vector3d(45, 45, 0)
        }
    }

    // View2, importScene with node id of external component + content inside
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
        Component.onCompleted: {
            // Currently this doesn't do anything
            importScene = null;
        }
        Model {
            // Behind, so partly visible
            source: "#Cube"
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            y: 50
            z: -120
            materials: DefaultMaterial {
                diffuseColor: "red"
            }
            eulerRotation: Qt.vector3d(45, 45, 45)
        }
        Model {
            // Above, so fully visible
            source: "#Cube"
            scale: Qt.vector3d(0.1, 0.1, 0.1)
            y: -10
            z: 120
            materials: DefaultMaterial {
                diffuseColor: "blue"
            }
            eulerRotation: Qt.vector3d(45, 45, 0)
        }
    }

    // View3, importScene with external component + content inside
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
            opacity: 0.8
        }
        Model {
            // Behind, so partly visible
            source: "#Cube"
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            y: 50
            z: -120
            materials: DefaultMaterial {
                diffuseColor: "red"
            }
            eulerRotation: Qt.vector3d(45, 45, 45)
        }
        Model {
            // Above, so fully visible
            source: "#Cube"
            scale: Qt.vector3d(0.1, 0.1, 0.1)
            y: -10
            z: 120
            materials: DefaultMaterial {
                diffuseColor: "blue"
            }
            eulerRotation: Qt.vector3d(45, 45, 0)
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
            opacity: 0.8
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
        Model {
            // Behind, so partly visible
            source: "#Cube"
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            y: 50
            z: -120
            materials: DefaultMaterial {
                diffuseColor: "red"
            }
            eulerRotation: Qt.vector3d(45, 45, 45)
        }
        Model {
            // Above, so fully visible
            source: "#Cube"
            scale: Qt.vector3d(0.1, 0.1, 0.1)
            y: -10
            z: 120
            materials: DefaultMaterial {
                diffuseColor: "blue"
            }
            eulerRotation: Qt.vector3d(45, 45, 0)
        }
    }
}
