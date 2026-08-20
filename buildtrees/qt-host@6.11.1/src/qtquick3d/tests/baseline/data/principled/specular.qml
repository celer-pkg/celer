// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 400
    height: 400
    color: "lightgray"

    View3D {
        anchors.fill: parent
        camera: camera
        renderMode: View3D.Offscreen

        OrthographicCamera {
            id: camera
            z: 500
        }

        DirectionalLight {
            id: dirLightX
            ambientColor: Qt.rgba(0.5, 0.5, 0.5, 1.0);
        }

        Texture {
            id: tex_rgba
            source: "../shared/maps/rgba.png"
        }

        Texture {
            id: tex_photo
            source: "../shared/maps/oulu_2.jpeg"
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

        Node {
            id: row1

            // Row 1
            Model {
                source: "#Sphere"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(-125, 125, 0)
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        roughness: .1
                        specularAmount: 0
                    } ]
            }

            Model {
                source: "#Sphere"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(0, 125, 0)
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        roughness: .1
                        specularAmount: .3
                    } ]
            }

            Model {
                source: "#Sphere"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(125, 125, 0)
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        roughness: .1
                        specularAmount: 1
                    } ]
            }
        }

        Node {
            id: row2

            // Row 2
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(-125, 0, 0)
                eulerRotation.y: 350
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        roughness: 1
                        specularAmount: 1
                        specularMap: tex_rgba
                    } ]
            }

            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(0, 0, 0)
                eulerRotation.y: 340
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        roughness: 1
                        specularAmount: .5
                        specularMap: tex_rgba
                    } ]
            }

            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(125, 0, 0)
                eulerRotation.y: 320
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        roughness: 1
                        specularAmount: 0
                        specularMap: tex_rgba
                    } ]
            }
        }

        // Row 3
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, -125, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    roughness: .1
                    specularAmount: 1
                } ]
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, -125, 0)
            eulerRotation.y: 350
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    roughness: 1
                    specularAmount: 1
                } ]
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, -125, 0)
            eulerRotation.y: 90
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    roughness: 1
                    specularAmount: 1
                } ]
        }
    }
}
