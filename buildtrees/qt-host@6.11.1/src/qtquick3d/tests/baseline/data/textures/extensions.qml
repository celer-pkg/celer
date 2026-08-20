// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick3D

Rectangle {
    width: 400
    height: 400
    color: "lightgray"

    // Row 1 - Quick loading extensionless texture file paths
    Item {
        height: 130
        width: parent.width

        Image {
            anchors.fill: parent
            source: "../shared/maps/checkers1.png"
            fillMode: Image.Tile
        }

        RowLayout {
            anchors.fill: parent

            Image {
                id: path1
                source: "../shared/maps/t2"
                fillMode: Image.Stretch
                Layout.alignment: Qt.AlignCenter
            }

            Image {
                id: path2
                source: "../shared/maps/t2_bc2"
                Layout.alignment: Qt.AlignCenter
            }

            Image {
                id: path3
                // Show that the .ktx file is preferred over the .png
                source: "../shared/maps/o1_bc1"
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

        // Row 2 - Quick3D loading extensionless texture file paths
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-125, 65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: path1.source }
                } ]
        }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(0, 65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: path2.source }
                } ]
        }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(125, 65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: path3.source }
                } ]
        }

        // Row 3 - Quick3D loading faulty paths
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-125, -65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: "../shared/maps/no-such-file.png" }
                } ]
        }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(0, -65, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: "../shared/maps/corrupt.png" }
                } ]
        }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(125, -65, 0)
            materials: [ DefaultMaterial {
                    // Does not exist either, but a jpeg file with the same basename does
                    diffuseMap: Texture { source: "../shared/maps/oulu_2.ktx" }
                } ]
        }
    }
}
