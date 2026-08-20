// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    id: sceneRoot

    property alias mainCamera: mainCamera

    DirectionalLight {
        brightness: 0.4
        color: "#f0e0a0"
    }
    DirectionalLight {
        brightness: 0.4
        position: Qt.vector3d(-200,-200,-500)
        eulerRotation: Qt.vector3d(20,-20,20)
        color: "#f0e0a0"
    }

    PerspectiveCamera {
        id: mainCamera
        x: 0
        z: 600
        SequentialAnimation on position.y {
            loops: Animation.Infinite
            NumberAnimation {
                duration: 6000
                easing.type: Easing.InOutQuad
                to: -80
            }
            NumberAnimation {
                duration: 3000
                easing.type: Easing.InOutQuad
                to: 80
            }
        }
    }

    Model {
        id: cube
        source: "#Cube"
        materials: DefaultMaterial {
        }

        NumberAnimation {
            target: cube
            property: "eulerRotation.x"
            duration: 3000
            easing.type: Easing.InOutQuad
            loops: Animation.Infinite
            running: true
            from: 0
            to: 360
        }

        SequentialAnimation on position.z {
            running: true
            loops: Animation.Infinite
            NumberAnimation {
                duration: 3000
                easing.type: Easing.InOutQuad
                to: 300
            }
            NumberAnimation {
                duration: 6000
                easing.type: Easing.InOutQuad
                to: 0
            }
            PauseAnimation {
                duration: 3000
            }
        }
    }

    Model {
        id: cone
        source: "#Cone"
        z: -250
        x: -150
        materials: DefaultMaterial {
        }
        SequentialAnimation on x {
            running: true
            loops: Animation.Infinite

            NumberAnimation {
                duration: 5000
                easing.type: Easing.InOutQuad
                to: 250
            }
            NumberAnimation {
                duration: 5000
                easing.type: Easing.InOutQuad
                to: -250
            }
        }
    }

    Model {
        id: cylinder
        source: "#Cylinder"
        z: 250
        x: 150
        materials: DefaultMaterial {
        }

        SequentialAnimation on x {
            running: true
            loops: Animation.Infinite

            NumberAnimation {
                duration: 5000
                easing.type: Easing.InOutQuad
                to: -150
            }
            NumberAnimation {
                duration: 5000
                easing.type: Easing.InOutQuad
                to: 150
            }
        }
    }
    Node {
        y: 260
        z: -100
        Text {
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 36
            font.bold: true
            text: "Press 0..4 to change the stereoscopic mode"
        }
    }
    Node {
        y: -260
        z: -100
        Text {
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 36
            font.bold: true
            text: "Press -/+ to change the eye separation"
        }
    }
}
