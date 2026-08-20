// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    width: 800
    height: 600

    Item {
        id: quickScene
        visible: false
        width: 1024
        height: 1024
        Image {
            source: "../shared/maps/00455_OpenfootageNET_field_low_resized.png"
            anchors.fill: parent
        }
        Text {
            text: "Background"
            font.pointSize: 36
            anchors.horizontalCenter: parent.horizontalCenter
            color: "red"
            y: 200
        }
    }

    View3D {
        anchors.fill: parent
        PerspectiveCamera {
            id: camera
            // Zoom out just so that we can see the Qt Quick rendered Text on
            // top of the Image.
            z: 2500
        }
        DirectionalLight {
        }
        Model {
            source: "../shared/models/InvertedSphere.mesh"
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
    }
}
