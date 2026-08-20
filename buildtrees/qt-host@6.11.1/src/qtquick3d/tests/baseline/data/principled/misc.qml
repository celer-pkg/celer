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
            id: dirLight1
            eulerRotation.y: 60
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

            // Row 1 - opacity
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(-125, 125, 0)
                eulerRotation.y: 60
                materials: [ PrincipledMaterial {
                        baseColorMap: tex_rgba
                        metalness: 0
                        opacity: .5
                    } ]
            }

            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(0, 125, 0)
                eulerRotation.y: 60
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        opacityMap: tex_rgba
                    } ]
            }

            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(125, 125, 0)
                eulerRotation.y: 60
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        opacity: .5
                        opacityMap: tex_rgba
                    } ]
            }
        }

        Node {
            id: row2

            // Row 2 - emissive
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(-125, 0, 0)
                eulerRotation.y: 60
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        emissiveFactor: Qt.vector3d(0.0, 1.0, 0.0) // green
                    } ]
            }

            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(0, 0, 0)
                eulerRotation.y: 60
                materials: [ PrincipledMaterial {
                        baseColorMap: tex_rgba
                        metalness: 0
                        emissiveFactor: Qt.vector3d(0.0, 1.0, 0.0) // green
                    } ]
            }

            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                position: Qt.vector3d(125, 0, 0)
                eulerRotation.y: 60
                materials: [ PrincipledMaterial {
                        baseColor: "#4080A0"
                        metalness: 0
                        emissiveFactor: Qt.vector3d(1.0, 1.0, 0.0) // yellow
                        emissiveMap: tex_rgba
                    } ]
            }
        }

        // Row 3 - specular tint
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, -125, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    roughness: 1
                    specularAmount: .7
                    specularTint: 0
                } ]
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, -125, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    roughness: 1
                    specularAmount: .7
                    specularTint: .5
                } ]
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, -125, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    roughness: 1
                    specularAmount: .7
                    specularTint: 1
                } ]
        }
    }
}
