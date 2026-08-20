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

        PerspectiveCamera {
            z: 600
        }

        DirectionalLight {
            eulerRotation.x: -45
            castsShadow: true
        }

        Model {
            source: "#Rectangle"
            materials: PrincipledMaterial {
                baseColor: "gray"
            }
            z: -100
            scale: Qt.vector3d(10, 10, 10)
        }

        Texture {
            id: tex_rgba
            source: "../shared/maps/alphaMode_rgba.png"
        }

        // top-left corner: alphaMode=Default (shadow should be visible)
        Model {
            source: "#Cube"
            position: Qt.vector3d(-200, 200, 0)
            materials: PrincipledMaterial {
                baseColorMap: tex_rgba
            }
        }

        // top-right corner: alphaMode=Blend (no shadow should be visible)
        Model {
            source: "#Cube"
            position: Qt.vector3d(200, 200, 0)
            materials: PrincipledMaterial {
                baseColorMap: tex_rgba
                alphaMode: PrincipledMaterial.Blend
            }
        }

        // bottom-left corner: alphaMode=Opaque (shadow should be visible)
        Model {
            source: "#Cube"
            position: Qt.vector3d(-200, -200, 0)
            materials: PrincipledMaterial {
                baseColorMap: tex_rgba
                alphaMode: PrincipledMaterial.Opaque
            }
        }

        // bottom-right corner: alphaMode=Mask (shadow should be visible)
        Model {
            source: "#Cube"
            position: Qt.vector3d(200, -200, 0)
            materials: PrincipledMaterial {
                baseColorMap: tex_rgba
                alphaMode: PrincipledMaterial.Mask
                // alphaCutoff = default(0.5)
            }
        }
    }
}
