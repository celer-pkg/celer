// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

// Tests that materialMode: RenderPass.OverrideMaterial works correctly inside
// a SubRenderPass. The sphere and cube have blue/red materials, but the
// sub-pass should override them with a flat green, so the output must be green.

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        anchors.fill: parent
        renderOverrides: View3D.DisableInternalPasses

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight { }

        RenderPassTexture { id: colorBuffer; format: RenderPassTexture.RGBA16F }

        DefaultMaterial {
            id: greenMat
            lighting: DefaultMaterial.NoLighting
            diffuseColor: "green"
        }

        RenderPass {
            id: mainPass
            clearColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
            commands: [
                ColorAttachment { target: colorBuffer },
                DepthStencilAttachment { },

                // Sub-pass overrides all materials with flat green.
                // If the fix is correct the output will be green;
                // if broken the output will use the original blue/red materials.
                SubRenderPass {
                    renderPass: RenderPass {
                        materialMode: RenderPass.OverrideMaterial
                        overrideMaterial: greenMat
                        commands: [
                            PipelineStateOverride { depthTestEnabled: false }
                        ]
                    }
                }
            ]
        }

        SimpleQuadRenderer {
            texture: Texture {
                textureProvider: RenderOutputProvider {
                    textureSource: RenderOutputProvider.UserPassTexture
                    renderPass: mainPass
                }
            }
        }

        Model {
            source: "#Cube"
            x: -100
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            eulerRotation.y: 30
            materials: PrincipledMaterial { baseColor: "blue" }
        }
        Model {
            source: "#Sphere"
            x: 100
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            materials: PrincipledMaterial { baseColor: "red" }
        }
    }
}
