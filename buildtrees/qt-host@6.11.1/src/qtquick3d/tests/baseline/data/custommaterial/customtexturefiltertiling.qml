// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: "black"

    View3D {
        id: v3d
        anchors.fill: parent
        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        property variant cubeScale: Qt.vector3d(1.5, 1.5, 1.5)

        // The default wrap mode is Repeat. Try with a shader that samples so that
        // it relies on the repeat wrap mode.
        Model {
            scale: v3d.cubeScale
            x: -200
            source: "#Cube"
            materials: [
                CustomMaterial {
                    TextureInput {
                        id: texInput
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                        }
                    }
                    fragmentShader: "customtexturefiltertiling_repeat.frag"
                    property TextureInput tex: texInput
                }
            ]
        }

        // Try ClampToEdge with the same shader, should sample zeroes, resulting in a grayish cube.
        Model {
            scale: v3d.cubeScale
            x: 0
            source: "#Cube"
            materials: [
                CustomMaterial {
                    fragmentShader: "customtexturefiltertiling_repeat.frag"
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                            tilingModeHorizontal: Texture.ClampToEdge
                            tilingModeVertical: Texture.ClampToEdge
                        }
                    }
                }
            ]
        }

        // ClampToEdge with in-range texture coordinates, should show up normally
        Model {
            scale: v3d.cubeScale
            x: 200
            source: "#Cube"
            materials: [
                CustomMaterial {
                    fragmentShader: "customtexturefiltertiling.frag"
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                            tilingModeHorizontal: Texture.ClampToEdge
                            tilingModeVertical: Texture.ClampToEdge
                        }
                    }
                }
            ]
        }

        // min/mag filter
        Model {
            scale: v3d.cubeScale
            x: -200
            y: -200
            source: "#Cube"
            materials: [
                CustomMaterial {
                    fragmentShader: "customtexturefiltertiling.frag"
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                            minFilter: Texture.Nearest
                            magFilter: Texture.Nearest
                        }
                    }
                }
            ]
        }

        Model {
            scale: v3d.cubeScale
            x: 0
            y: -200
            source: "#Cube"
            materials: [
                CustomMaterial {
                    fragmentShader: "customtexturefiltertiling.frag"
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                            minFilter: Texture.Linear
                            magFilter: Texture.Linear
                        }
                    }
                }
            ]
        }

        Model {
            scale: v3d.cubeScale
            x: 200
            y: -200
            source: "#Cube"
            materials: [
                CustomMaterial {
                    fragmentShader: "customtexturefiltertiling.frag"
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                            minFilter: Texture.None // effectively linear
                            magFilter: Texture.None
                        }
                    }
                }
            ]
        }

        // mipmap, common case
        Model {
            scale: v3d.cubeScale
            x: -200
            y: 200
            source: "#Cube"
            materials: [
                CustomMaterial {
                    fragmentShader: "customtexturefiltertiling.frag"
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                            minFilter: Texture.Linear
                            magFilter: Texture.Linear
                            mipFilter: Texture.Linear
                            generateMipmaps: true
                        }
                    }
                }
            ]
        }

        // missing generateMipmaps, not mipmapped
        Model {
            scale: v3d.cubeScale
            x: 0
            y: 200
            source: "#Cube"
            materials: [
                CustomMaterial {
                    fragmentShader: "customtexturefiltertiling.frag"
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                            minFilter: Texture.Nearest
                            magFilter: Texture.Nearest
                            mipFilter: Texture.Linear
                        }
                    }
                }
            ]
        }

        // Now actually sample a mip level in the shader. (textureLod)
        Model {
            scale: v3d.cubeScale
            x: 200
            y: 200
            source: "#Cube"
            materials: [
                CustomMaterial {
                    fragmentShader: "customtexturefiltertiling_mip.frag"
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                            minFilter: Texture.Nearest
                            magFilter: Texture.Nearest
                            mipFilter: Texture.Nearest
                            generateMipmaps: true
                        }
                    }
                }
            ]
        }
    }
}
