// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    width: 400
    height: 400
    visible: true

    View3D {
        id: origin
        anchors.fill: parent
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(-200, 300, 500)
            eulerRotation.x: -20
            eulerRotation.y: -20
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-135, -110, 0)
            brightness: 1
        }

        Node {
            position: Qt.vector3d(-20, 110, 250)
            eulerRotation.y: -90
            Text {
                anchors.centerIn: parent
                width: 150
                horizontalAlignment: Text.AlignJustify
                font.pixelSize: 12
                color: "#e0e0e0"
                style: Text.Raised
                text: "A declaration before \"nestedView3D\""
            }
        }

        Node {
            y: 150
            View3D {
                id: nestedView3D
                width: 100
                height: 100

                environment: SceneEnvironment {
                    clearColor: "red"
                    backgroundMode: SceneEnvironment.Color
                }
                Model{
                    source: "#Cube"
                    materials: DefaultMaterial {
                        diffuseColor: "green"
                    }
                }
                DirectionalLight { }

                PerspectiveCamera {
                    id: cam2
                    position: Qt.vector3d(200, 300, 500)
                    eulerRotation.x: -20
                    eulerRotation.y: 20
                }
            }
        }

        Node {
            position: Qt.vector3d(-150, 320, 250)
            eulerRotation.y: 40
            Text {
                anchors.centerIn: parent
                width: 150
                horizontalAlignment: Text.AlignJustify
                font.pixelSize: 12
                color: "#e0e0e0"
                style: Text.Raised
                text: "A declaration after \"nestedView3D\""
            }
        }
    }
}
