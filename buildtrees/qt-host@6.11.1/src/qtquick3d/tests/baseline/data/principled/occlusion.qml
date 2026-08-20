// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 400
    height: 480
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
            id: dirLight
            brightness: 1.5
            eulerRotation.y: 310
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

        Texture {
            id: tex_ibl
            source: "../shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
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

        // Row 1
        // occlusionMap exists but it does not appear.
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, 180, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    occlusionAmount: 0
                    occlusionMap: tex_rgba
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, 180, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    occlusionAmount: .5
                    occlusionMap: tex_rgba
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, 180, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 0
                    occlusionAmount: 1
                    occlusionMap: tex_rgba
                } ]
        }

        // Row 2
        // occlusionMap exists but it does not appear.
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, 60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColor: "#4080A0"
                    metalness: 1
                    roughness: 1
                    occlusionAmount: 1
                    occlusionMap: tex_rgba
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, 60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_photo
                    metalness: 0
                    occlusionAmount: .5
                    occlusionMap: tex_rgba
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, 60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_photo
                    metalness: 0
                    occlusionAmount: 1
                    occlusionMap: tex_rgba
                } ]
        }

        // Row 3 - same as the Row 1 with lightProbe
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, -60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    lightProbe: tex_ibl
                    baseColor: "#4080A0"
                    metalness: 0
                    occlusionAmount: 0
                    occlusionMap: tex_rgba
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, -60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    lightProbe: tex_ibl
                    baseColor: "#4080A0"
                    metalness: 0
                    occlusionAmount: .5
                    occlusionMap: tex_rgba
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, -60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    lightProbe: tex_ibl
                    baseColor: "#4080A0"
                    metalness: 0
                    occlusionAmount: 1
                    occlusionMap: tex_rgba
                } ]
        }

        // Row 4 same as the Row 2 with lightProbe
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, -180, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    lightProbe: tex_ibl
                    baseColor: "#4080A0"
                    metalness: 1
                    roughness: 1
                    occlusionAmount: 1
                    occlusionMap: tex_rgba
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, -180, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    lightProbe: tex_ibl
                    baseColorMap: tex_photo
                    metalness: 0
                    occlusionAmount: .5
                    occlusionMap: tex_rgba
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, -180, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    lightProbe: tex_ibl
                    baseColorMap: tex_photo
                    metalness: 0
                    occlusionAmount: 1
                    occlusionMap: tex_rgba
                } ]
        }
    }
}
