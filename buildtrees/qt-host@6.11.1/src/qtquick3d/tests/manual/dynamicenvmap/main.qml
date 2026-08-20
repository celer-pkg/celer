// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Item {
    Item {
        id: quickScene
        visible: false
        width: 1024
        height: 1024
        Image {
            source: "00455_OpenfootageNET_field_low_resized.jpg"
        }
        Text {
            text: "Background"
            font.pointSize: 36
            anchors.horizontalCenter: parent.horizontalCenter
            y: 200
        }
        Rectangle {
            width: 128
            height: 128
            anchors.horizontalCenter: parent.horizontalCenter
            y: 300
            color: "red"
            NumberAnimation on rotation {
                from: 0; to: 360; duration: 3000; loops: -1
            }
        }
    }

    View3D {
        anchors.fill: parent
        PerspectiveCamera {
            id: camera
            z: 600
        }
        DirectionalLight {
        }
        Model {
            source: "InvertedSphere.mesh"
            scale: Qt.vector3d(4000, 4000, 4000)
            eulerRotation.y: 90
            materials: PrincipledMaterial {
                lighting: PrincipledMaterial.NoLighting
                baseColorMap: Texture {
                    flipU: true
                    sourceItem: quickScene
                }
            }
        }
        Model {
            source: "#Cube"
            scale: Qt.vector3d(3, 3, 3)
            eulerRotation: Qt.vector3d(30, 30, 0)
            materials: PrincipledMaterial {
                specularReflectionMap: Texture {
                    sourceItem: quickScene
                    mappingMode: Texture.Environment
                }
            }
        }
        WasdController {
            controlledObject: camera
        }
    }
}
