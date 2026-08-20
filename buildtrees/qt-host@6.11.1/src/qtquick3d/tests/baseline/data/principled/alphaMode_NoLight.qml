// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 640
    height: 640

    View3D {
        anchors.fill: parent
        camera: camera
        renderMode: View3D.Offscreen

        OrthographicCamera {
            id: camera
            z: 50
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

        Texture {
            id: tex_rgba
            source: "../shared/maps/alphaMode_rgba.png"
        }

        // alphaMode == Opaque
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(3, 3, 1)
            position: Qt.vector3d(-160, 160, 0)
            materials: [ PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Opaque
                } ]
        }

        // alphaMode == Mask && alphaCutoff = 0.25
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(3, 3, 1)
            position: Qt.vector3d(160, 160, 0)
            materials: [ PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.25
                } ]
        }

        // alphaMode == Mask && alphaCutoff = default(0.5)
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(3, 3, 1)
            position: Qt.vector3d(-160, -160, 0)
            materials: [ PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                } ]
        }

        // alphaMode == Mask && alphaCutoff = 0.75
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(3, 3, 1)
            position: Qt.vector3d(160, -160, 0)
            materials: [ PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.75
                } ]
        }
    }
}
