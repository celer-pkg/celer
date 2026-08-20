// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 400
    height: 500
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

        // Row 1, just baseColor with alpha == 64
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, 175, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColor: "#404080A0"
                    metalness: 0
                    alphaMode: PrincipledMaterial.Default
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, 175, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColor: "#404080A0"
                    metalness: 0
                    alphaMode: PrincipledMaterial.Blend
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, 175, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColor: "#404080A0"
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                } ]
        }

        // Row 2, use a base color map
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, 60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: .5
                    alphaMode: PrincipledMaterial.Default // no blend, look at the background of the letter R f.ex.
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, 60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Blend
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, 60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask // some things are now cut, e.g. look at the background behind the G
                } ]
        }

        // Row 3, base color map, with custom alphaCutoff
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, -60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0 // alpha changes to 1 everywhere
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, -60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 1 // all but the fully opaque pixels are cut
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, -60, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Blend
                    alphaCutoff: 1.0 // no effect due to alphaMode
                } ]
        }

        // Row 4, now also have a custom base color
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(-125, -175, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    baseColor: "#A04080A0"
                    metalness: 0
                    alphaMode: PrincipledMaterial.Default // alpha is combined but no blending (look for cyan behind R)
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(0, -175, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    baseColor: "#A04080A0"
                    metalness: 0
                    alphaMode: PrincipledMaterial.Blend // now blended with the gray background
                } ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            position: Qt.vector3d(125, -175, 0)
            eulerRotation.y: 60
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    baseColor: "#A04080A0"
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                } ]
        }
    }
}
