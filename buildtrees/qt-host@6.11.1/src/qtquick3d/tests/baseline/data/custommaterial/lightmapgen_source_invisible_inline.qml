// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 800
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: lightmapSource

        // This View3D will not be visible on the screen but the content is still
        // usable as a texture because it is a Quick item layer, which allows it to
        // function in ways a visible: false item cannot.
        layer.enabled: true
        visible: false

        // The special thing about this test is here: instead of going with
        // Offscreen, switch to Inline, after all this is a Quick item layer so
        // there is a texture regardless. (and the benefit is that we avoid a
        // textured quad drawing)
        renderMode: View3D.Inline

        anchors.fill: parent

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
        anchors.fill: parent

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
                vertexShader: "lightmapgen_use_inline.vert"
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
