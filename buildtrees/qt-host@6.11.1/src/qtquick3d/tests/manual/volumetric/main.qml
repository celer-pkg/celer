// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls
import Volume

Window {
    id: window
    width: 640
    height: 640
    visible: true
    color: "black"

    WasdController {
        controlledObject: camera
    }

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: window.color
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                source: "qrc:///OpenfootageNET_garage-1024.hdr"
            }
            probeOrientation: Qt.vector3d(0, -90, 0)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 400)
        }

        DirectionalLight {
            eulerRotation.x: -60
            castsShadow: true
        }

        Model {
            source: "#Cube"
            materials: CustomMaterial {
                shadingMode: CustomMaterial.Unshaded
                vertexShader: "custom.vert"
                fragmentShader: "custom.frag"
                property TextureInput volume: TextureInput {
                    enabled: true
                    texture: Texture {
                        textureData: VolumeTextureData {}
                    }
                }
                property TextureInput colormap: TextureInput {
                    enabled: true
                    texture: Texture { source: "colormap.png" }
                }
                sourceBlend: CustomMaterial.SrcAlpha
                destinationBlend: CustomMaterial.OneMinusSrcAlpha
            }
        }
    }

    DebugView {
        anchors.right: parent.right
        source: view
    }
}
