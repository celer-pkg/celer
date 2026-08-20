// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: v3d
        anchors.fill: parent
        renderOverrides: View3D.DisableInternalPasses

        environment: SceneEnvironment {
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        Model {
            source: "#Cube"
            x: -150
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation.x: 30
            eulerRotation.y: 20
            materials: [
                PrincipledMaterial {
                    baseColor: "red"
                }
            ]
        }
        Model {
            source: "#Sphere"
            x: 150
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation.x: 30
            eulerRotation.y: 20
            materials: [
                PrincipledMaterial {
                    baseColor: "yellow"
                }
            ]
        }

        CustomMaterial {
            id: customMaterial
            shadingMode: CustomMaterial.Unshaded
            vertexShader: "overridematerial.vert"
            fragmentShader: "overridematerial.frag"
        }


        RenderPass {
            id: renderpass
            clearColor: Qt.rgba(0.0, 0.0, 0.0, 1.0)
            depthClearValue: 1.0
            materialMode: RenderPass.OverrideMaterial
            overrideMaterial: customMaterial

            RenderPassTexture {
                id: colorbuffer
                format: RenderPassTexture.RGBA16F
            }

            RenderPassTexture {
                id: depthbuffer
                format: RenderPassTexture.Depth32
            }

            commands: [
                ColorAttachment { target: colorbuffer },
                DepthTextureAttachment { target: depthbuffer },
            ]
        }
        RenderOutputProvider {
            id: renderpassProvider
            textureSource: RenderOutputProvider.UserPassTexture
            renderPass: renderpass
            attachmentSelector: RenderOutputProvider.Attachment0
        }

        SimpleQuadRenderer {
            texture: Texture {
                textureProvider: renderpassProvider
            }
        }
    }
}
