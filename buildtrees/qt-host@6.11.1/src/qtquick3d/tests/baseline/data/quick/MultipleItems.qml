// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(1, 1, 0.5, 1)

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        Node {
            x: -200
            y: -200
            /*NumberAnimation on z {
                running: true
                to: 100
                duration: 1500
            }*/
            Text {
                text: "Text before Rectangle"
                color: "red"
                font.pixelSize: 40
            }
            Rectangle {
                width: 100
                height: 200
                color: "green"
            }
        }

        Node {
            x: -200
            y: 200
            Rectangle {
                width: 100
                height: 200
                color: "blue"
            }
            Text {
                text: "Text after Rectangle"
                color: "red"
                font.pixelSize: 40
            }
        }

        Node {
            x: 200
            y: 200
            Rectangle {
                width: 200
                height: 200
                color: "green"
                // rotation and transform ignored
                // because Rectangle parent is Node
                rotation: 30
                transform: Rotation {
                    origin.x: 30
                    origin.y: 30
                    axis { x: 0; y: 1; z: 0 }
                    angle: 45
                }
            }
        }

        Node {
            x: 200
            y: 200
            z: -10
            Text {
                text: "Node with Text further away"
                color: "red"
                font.pixelSize: 30
            }
        }

        Node {
            x: 200
            y: -200
            z: -10
            Rectangle {
                width: 200
                height: 200
                color: "black"
            }
        }

        Node {
            x: 200
            y: -200
            Item {
                width: 300
                height: 300
                Rectangle {
                    anchors.centerIn: parent
                    width: 200
                    height: 200
                    color: "green"
                    // rotation and transform respected
                    // because Rectangle parent is Item
                    rotation: 30
                    transform: Rotation {
                        origin.x: 40
                        origin.y: 40
                        axis { x: 0; y: 1; z: 0 }
                        angle: 25
                    }
                }
            }
        }

        Node {
            z: 100
            Image {
                width: 100
                height: 100
                source: "../shared/maps/checkerboard_1.png"
            }
        }
    }
}
