// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick3D

Rectangle {
    width: 400
    height: 400
    color: "lightgray"

    // Row 1 - Quick
    Image {
        height: 130
        width: parent.width
        source: "../shared/maps/checkers1.png"
        fillMode: Image.Tile

        RowLayout {
            anchors.fill: parent

            Image {
                id: o1_bc1
                source: "../shared/maps/o1_bc1.ktx"
                Layout.alignment: Qt.AlignCenter
            }

            Image {
                id: t2_bc2
                source: "../shared/maps/t2_bc2.ktx"
                Layout.alignment: Qt.AlignCenter
            }

            Image {
                id: t2_png
                source: "../shared/maps/t2.png"
                Layout.alignment: Qt.AlignCenter

            }
        }
    }

    View3D {
        y: 140
        width: parent.width
        height: parent.height - y
        renderMode: View3D.Offscreen

        OrthographicCamera {
            z: 500
        }

        DirectionalLight {
            eulerRotation.y: 180
            ambientColor: Qt.rgba(1.0, 1.0, 1.0, 1.0);
        }

        Model {
            source: "#Rectangle"
            materials: [ DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                    diffuseMap: Texture {
                        source: "../shared/maps/checkers1.png"
                        tilingModeHorizontal: Texture.Repeat
                        tilingModeVertical: Texture.Repeat
                        scaleU: 100
                        scaleV: 100
                    }
                } ]
            z: -500
            scale: Qt.vector3d(10, 10, 1)
        }

        // Row 2 - Quick3D loading texture files
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-125, 65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: o1_bc1.source }
                } ]
        }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(0, 65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: t2_bc2.source }
                } ]
        }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(125, 65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: t2_png.source }
                } ]
        }

        // Row 3 - Quick3D loading textures from Quick items
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-125, -65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { sourceItem: o1_bc1 }
                } ]
        }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(0, -65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { sourceItem: t2_bc2 }
                } ]
        }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(125, -65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { sourceItem: t2_png }
                } ]
        }
    }
}
