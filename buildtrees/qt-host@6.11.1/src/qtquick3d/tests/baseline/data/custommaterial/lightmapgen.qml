// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick3D.Effects
import QtQuick

Rectangle {
    id: root
    property int api: GraphicsInfo.api
    width: 800
    height: 800
    color: Qt.rgba(0, 0, 0, 1)

    Flip {
        id: vFlipEffect
        flipHorizontally: false
        flipVertically: true
    }

    View3D {
        id: lightmapSource
        renderMode: View3D.Offscreen
        width: parent.width / 2
        height: parent.height / 2

        environment: SceneEnvironment {
            clearColor: "transparent"
            backgroundMode: SceneEnvironment.Color

            // To get identical results on-screen with all graphics APIs. This (and the
            // corresponding V flip in the other view) would _not_ be needed at all if we
            // didn't want to visualize the lightmap on screen.
            effects: root.api === GraphicsInfo.OpenGL ? [ vFlipEffect ] : []
        }

        DirectionalLight {
        }

        PerspectiveCamera {
            id: camera1
            z: 200
        }

        Model {
            source: "../shared/models/animal_with_lightmapuv1.mesh"
            scale: Qt.vector3d(20, 20, 20)
            materials: CustomMaterial {
                shadingMode: CustomMaterial.Shaded
                vertexShader: "lightmapgen.vert"
                fragmentShader: "lightmapgen.frag"
                cullMode: Material.NoCulling
            }
        }
    }

    View3D {
        width: parent.width / 2
        height: parent.height / 2
        x: parent.width / 2
        y: parent.height / 2

        environment: SceneEnvironment {
            clearColor: "white"
            backgroundMode: SceneEnvironment.Color
        }

        camera: camera1

        Model {
            source: "../shared/models/animal_with_lightmapuv1.mesh"
            scale: Qt.vector3d(20, 20, 20)
            materials: CustomMaterial {
                shadingMode: CustomMaterial.Unshaded
                vertexShader: "lightmapgen_use.vert"
                fragmentShader: "lightmapgen_use.frag"
                property TextureInput tex: TextureInput {
                    texture: Texture {
                        sourceItem: lightmapSource
                        minFilter: Texture.Nearest
                        magFilter: Texture.Nearest
                    }
                }
            }
        }
    }
}
