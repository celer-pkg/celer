// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(1, 1, 1, 1)

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
            x: -300
            y: 250
            Text {
                text: "Text1 - visible: false"
                visible: false
                font.pixelSize: 40
            }
        }
        Node {
            x: 300
            y: 250
            Text {
                text: "Text2 - visible -> false"
                font.pixelSize: 40
                visible: true
                Component.onCompleted: {
                    visible = false;
                }
            }
        }
        Node {
            x: -300
            y: 150
            Text {
                text: "Text3 - visible -> true"
                font.pixelSize: 40
                visible: false
                Component.onCompleted: {
                    visible = true;
                }
            }
        }
        Node {
            x: 300
            y: 150
            Text {
                opacity: 0
                text: "Text4 - opacity: 0"
                font.pixelSize: 40
            }
        }
        Node {
            x: -300
            y: 50
            Text {
                NumberAnimation on opacity {
                    running: true
                    to: 0
                    duration: 500
                }
                text: "Text5 - opacity -> 0"
                font.pixelSize: 40
            }
        }
        Node {
            x: 300
            y: 50
            Text {
                NumberAnimation on opacity {
                    running: true
                    to: 1
                    duration: 500
                }
                text: "Text6 - opacity -> 1"
                opacity: 0
                font.pixelSize: 40
            }
        }
        Node {
            x: -300
            y: -50
            visible: false
            Text {
                text: "Text7 - Node visible: false"
                font.pixelSize: 40
            }
        }
        Node {
            x: 300
            y: -50
            visible: true
            Component.onCompleted: {
                visible = false;
            }
            Text {
                text: "Text8 - Node visible -> false"
                font.pixelSize: 40
            }
        }
        Node {
            x: -300
            y: -150
            visible: false
            Component.onCompleted: {
                visible = true;
            }
            Text {
                text: "Text9 - Node visible -> true"
                font.pixelSize: 40
            }
        }
        Node {
            x: 300
            y: -150
            opacity: 0
            Text {
                text: "Text10 - Node opacity: 0"
                font.pixelSize: 40
            }
        }
        Node {
            x: -300
            y: -250
            opacity: 1
            NumberAnimation on opacity {
                running: true
                to: 0
                duration: 500
            }
            Text {
                text: "Text11 - Node opacity -> 0"
                font.pixelSize: 40
            }
        }
        Node {
            x: 300
            y: -250
            opacity: 0
            NumberAnimation on opacity {
                running: true
                to: 1
                duration: 500
            }
            Text {
                text: "Text12 - Node opacity -> 1"
                font.pixelSize: 40
            }
        }
    }
}
