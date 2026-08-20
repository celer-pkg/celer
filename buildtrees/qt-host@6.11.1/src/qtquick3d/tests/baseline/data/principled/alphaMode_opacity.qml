// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 500
    height: 400

    View3D {
        anchors.fill: parent
        camera: camera
        renderMode: View3D.Offscreen

        OrthographicCamera {
            id: camera
            z: 50
        }

        DirectionalLight {
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

        // Row 1: alphaMode == Opaque
        // Col 1: Node opacity = default(1.0), Material opacity = 0.7
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(-200, 150, 0)
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Opaque
                    opacity: 0.7
                } ]
        }
        // Col 2: Node opacity = 0.7, Material opacity = default(1.0)
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(-100, 150, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Opaque
                } ]
        }
        // Col 3: Node opacity = 0.7, Material opacity = 0.7
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, 150, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Opaque
                    opacity: 0.7
                } ]
        }
        // Col 4: Node opacity = 0.0, Material opacity = 0.7
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(100, 150, 0)
            opacity: 0
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Opaque
                    opacity: 0.7
                } ]
        }
        // Col 5: Node opacity = 0.7, Material opacity = 0.0
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, 150, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Opaque
                    opacity: 0
                } ]
        }

        // Row 2: alphaMode == Mask && alphaCutoff = 0.25
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(-200, 50, 0)
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.25
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(-100, 50, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.25
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, 50, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.25
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(100, 50, 0)
            opacity: 0
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.25
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(200, 50, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.25
                    opacity: 0
                } ]
        }
        // Row 3: alphaMode == Mask && alphaCutoff = default(0.5)
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(-200, -50, 0)
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(-100, -50, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, -50, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(100, -50, 0)
            opacity: 0
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(200, -50, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    opacity: 0
                } ]
        }
        // Row 4: alphaMode == Mask && alphaCutoff = 0.75
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(-200, -150, 0)
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.75
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(-100, -150, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.75
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, -150, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.75
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(100, -150, 0)
            opacity: 0
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.75
                    opacity: 0.7
                } ]
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(200, -150, 0)
            opacity: 0.7
            materials: [ PrincipledMaterial {
                    baseColorMap: tex_rgba
                    metalness: 0
                    alphaMode: PrincipledMaterial.Mask
                    alphaCutoff: 0.75
                    opacity: 0
                } ]
        }
    }
}
