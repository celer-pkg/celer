// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 600
    visible: true
    View3D {
        anchors.fill: parent
        camera: camera
        id: view3d

        environment: SceneEnvironment {
            clearColor: "steelblue"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            id: cameraNode
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 600)
            }
            eulerRotation.x: -45
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            ambientColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        }

        Component {
            id: referenceCube
            Node {
            Model {
                source: "#Cube"
                position: Qt.vector3d((index - 4) * 100, 0, 0)
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: DefaultMaterial {
                    diffuseColor: (index == 4) ? "red"
                                : (index % 2) ? "darkgray" : "lightgray"
                }
            }
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, (index + 1) * 100, 0)
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: DefaultMaterial {
                    diffuseColor: "white"
                }
            }
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 0, (index + 1) * 100)
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: DefaultMaterial {
                    diffuseColor: "black"
                }
            }
            }
        }

        Repeater3D {
            model: 10
            delegate: referenceCube
        }

        InstanceList {
            id: manualInstancing
            instances: [
            InstanceListEntry {
                position: Qt.vector3d(-250, 0, 0)
                color: "lightgreen"
            },
            InstanceListEntry {
                position: Qt.vector3d(-250, 0, 250)
                color: "yellow"
            },
            InstanceListEntry {
                position: Qt.vector3d(0, 0, 0)
                color: "lavender"
            },
            InstanceListEntry {
                position: Qt.vector3d(0, 250, 0)
                color: "cyan"
            },
            InstanceListEntry {
                position: Qt.vector3d(0, 0, 250)
                color: "lightblue"
            },
            InstanceListEntry {
                position: Qt.vector3d(250, 0, 0)
                color: "pink"
            },
            InstanceListEntry {
                position: Qt.vector3d(250, 250, 0)
                color: "salmon"
            }
            ]
        }

        Node {
            id: testroot
            scale: "0.5, 0.5, 0.5"
            x: 50
            Node {
                id: intermediate
                Model {
                    instancing: manualInstancing
                    instanceRoot: testroot
                    source: "#Cube"
                    materials: DefaultMaterial { diffuseColor: "white" }
                    opacity: 0.7
                }
            }
        }
    }
}
