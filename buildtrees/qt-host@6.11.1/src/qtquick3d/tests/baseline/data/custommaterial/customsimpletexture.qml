// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: "#444845"

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.2, 0.2, 0.2, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        property real time: 10
        property real amplitude: 4

        // texture disabled (result should be black)
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            x: -200
            materials: [
                CustomMaterial {
                    property alias time: v3d.time
                    property alias amplitude: v3d.amplitude
                    vertexShader: "customsimpletexture.vert"
                    fragmentShader: "customsimpletexture.frag"
                    property TextureInput tex1: TextureInput {
                        enabled: false
                    }
                }
            ]
        }

        // texture enabled but no actual Texture is given (should survive with
        // dummy texture, result is expected to be black)
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            materials: [
                CustomMaterial {
                    property alias time: v3d.time
                    property alias amplitude: v3d.amplitude
                    vertexShader: "customsimpletexture.vert"
                    fragmentShader: "customsimpletexture.frag"
                    property TextureInput tex1: TextureInput {
                        enabled: true
                    }
                }
            ]
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            x: 200
            materials: [
                CustomMaterial {
                    property alias time: v3d.time
                    property alias amplitude: v3d.amplitude
                    vertexShader: "customsimpletexture.vert"
                    fragmentShader: "customsimpletexture.frag"
                    property TextureInput tex1: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                        }
                    }
                }
            ]
        }

        // sample two textures
        Model {
            source: "#Cube"
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            y: -200
            materials: [
                CustomMaterial {
                    property real time: 0
                    property real amplitude: 0
                    vertexShader: "customsimpletexture.vert"
                    fragmentShader: "customsimpletexture2.frag"
                    property TextureInput tex1: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                        }
                    }
                    property TextureInput tex2: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/rgba.png"
                        }
                    }
                }
            ]
        }

        // not so simple after all: combine SCREEN_TEXTURE with another texture
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            y: 200
            materials: [
                CustomMaterial {
                    property real time: 0
                    property real amplitude: 0
                    vertexShader: "customsimpletexture.vert"
                    fragmentShader: "customsimpletexturescreen.frag"
                    property TextureInput tex1: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                        }
                    }
                }
            ]
        }
    }
}
