// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        anchors.fill: parent

        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
        }

        Model {
            y: 200
            scale: Qt.vector3d(2, 2, 1)
            source: "#Rectangle"

            DefaultMaterial {
                id: material0
                diffuseMap: material_diffusemap0
                Texture {
                    id: material_diffusemap0
                    source: "../shared/maps/oulu_2.jpeg"
                }
            }
            materials: [material0]
        }

        Model {
            x: -400
            y: -100
            scale: Qt.vector3d(3, 3, 1)
            source: "#Rectangle"

            DefaultMaterial {
                id: material1
                diffuseMap: material_diffusemap1
                Texture {
                    id: material_diffusemap1
                    source: "../shared/maps/oulu_2.jpeg"
                    flipU: true
                }
            }
            materials: [material1]
        }

        Model {
            x: 0
            y: -100
            scale: Qt.vector3d(3, 3, 1)
            source: "#Rectangle"

            DefaultMaterial {
                id: material2
                diffuseMap: material_diffusemap2
                Texture {
                    id: material_diffusemap2
                    source: "../shared/maps/oulu_2.jpeg"
                    flipV: true
                }
            }
            materials: [material2]
        }

        Model {
            x: 400
            y: -100
            scale: Qt.vector3d(3, 3, 1)
            source: "#Rectangle"

            DefaultMaterial {
                id: material3
                diffuseMap: material_diffusemap3
                Texture {
                    id: material_diffusemap3
                    source: "../shared/maps/oulu_2.jpeg"
                    flipU: true
                    flipV: true
                }
            }
            materials: [material3]
        }
    }
}
