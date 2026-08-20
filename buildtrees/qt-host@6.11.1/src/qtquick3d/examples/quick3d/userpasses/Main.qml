// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

ApplicationWindow {
    width: 1280
    height: 720
    visible: true
    title: qsTr("User Passes")



    // ![disable internal passes]
    View3D {
        id: view3D
        anchors.fill: parent
        renderOverrides: View3D.DisableInternalPasses
        environment: ExtendedSceneEnvironment {
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData {
                }
            }
            backgroundMode: SceneEnvironment.SkyBox
        }
    // ![disable internal passes]

        //! [main texture]
        SimpleQuadRenderer {
            texture: Texture {
                textureProvider: mainColorPassProvider
            }
        }

        RenderPassTexture {
            id: mainColorTexture
            format: RenderPassTexture.RGBA16F
        }

        RenderOutputProvider {
            id: mainColorPassProvider
            textureSource: RenderOutputProvider.UserPassTexture
            renderPass: mainColorPass
            attachmentSelector: RenderOutputProvider.Attachment0
        }
        //! [main texture]

        //! [depth texture]
        RenderPassTexture {
            id: mainDepthStencilTexture
            format: RenderPassTexture.Depth24Stencil8
        }
        //! [depth texture]

        Node {
            id: originNode
            y: 180

            PerspectiveCamera {
                id: cameraNode
                z: 300
            }
        }

        DirectionalLight {
            eulerRotation.x: -45
            castsShadow: true
        }

        // ![deferred lighting quad]
        Model {
            id: deferredLightingQuad
            layers: ContentLayer.Layer13
            castsShadows: false
            receivesShadows: false
            geometry: PlaneGeometry {
                // geometry doesn't matter, just need 4 verts
                plane: PlaneGeometry.XY
            }
            materials: [
                CustomMaterial {
                    id: lightingPassMaterial
                    property TextureInput gbuffer0: TextureInput {
                        enabled: true
                        texture: Texture {
                            textureProvider: gbuffer0Provider
                        }
                    }
                    property TextureInput gbuffer1: TextureInput {
                        enabled: true
                        texture: Texture {
                            textureProvider: gbuffer1Provider
                        }
                    }
                    property TextureInput gbuffer2: TextureInput {
                        enabled: true
                        texture: Texture {
                            textureProvider: gbuffer2Provider
                        }
                    }
                    shadingMode: CustomMaterial.Unshaded
                    fragmentShader: "lighting.frag"
                    vertexShader: "lighting.vert"
                }
            ]
        }
        // ![deferred lighting quad]

        // ![main color pass]
        RenderPass {
            id: mainColorPass
            clearColor: "black"
            // Preserve depth across SubRenderPasses so geometry depth is available
            // when rendering the skybox, transparent objects, and 2D items.
            renderTargetFlags: RenderPass.RenderTargetFlags.PreserveDepthStencilContents

            commands: [
                ColorAttachment {
                    target: mainColorTexture
                },
                DepthTextureAttachment {
                    target: mainDepthStencilTexture
                },
                RenderablesFilter {
                    // Nothing renders directly in the outer pass; all rendering
                    // is delegated to the SubRenderPasses below.
                    renderableTypes: RenderablesFilter.None
                },

                // 1. Deferred lighting: shade opaque geometry stored in the G-buffer.
                SubRenderPass {
                    renderPass: RenderPass {
                        id: deferredLightingPass
                        materialMode: RenderPass.OriginalMaterial
                        commands: [
                            PipelineStateOverride {
                                // The full-screen quad must not write or test depth;
                                // geometry depth was already written by the G-buffer pass.
                                depthWriteEnabled: false
                                depthTestEnabled: false
                            },
                            RenderablesFilter { layerMask: ContentLayer.Layer13 }
                        ]
                    }
                },

                // 2. Skybox: render the environment behind all scene geometry.
                SubRenderPass {
                    renderPass: RenderPass {
                        id: skyboxPass
                        passMode: RenderPass.SkyboxPass
                        commands: [
                            PipelineStateOverride {
                                // The skybox is rendered "at infinity" so it must
                                // depth-test (to be hidden by geometry) but must not
                                // write depth.
                                depthTestEnabled: true
                                depthWriteEnabled: false
                            }
                        ]
                    }
                },

                // 3. 2D items: render any Qt Quick Items embedded in the 3D scene.
                SubRenderPass {
                    renderPass: RenderPass {
                        id: item2DPass
                        passMode: RenderPass.Item2DPass
                    }
                },

                // 4. Transparent objects: render blended geometry on top of everything else.
                SubRenderPass {
                    renderPass: RenderPass {
                        id: transparentItemPass
                        materialMode: RenderPass.OriginalMaterial
                        commands: [
                            RenderablesFilter {
                                renderableTypes: RenderablesFilter.Transparent
                                layerMask: ContentLayer.Layer0 | ContentLayer.Layer1
                            },
                            PipelineStateOverride {
                                // Enable alpha blending and depth testing so transparent
                                // objects sort correctly against opaque geometry.
                                blendEnabled: true
                                depthTestEnabled: true
                                targetBlend0.enable: true
                                targetBlend0.srcColor: RenderTargetBlend.SrcAlpha
                                targetBlend0.dstColor: RenderTargetBlend.OneMinusSrcAlpha
                                targetBlend0.srcAlpha: RenderTargetBlend.One
                                targetBlend0.dstAlpha: RenderTargetBlend.OneMinusSrcAlpha
                            }
                        ]
                    }
                }
            ]
        }
        // ![main color pass]

        // ![GBufferPass usage]
        GBufferPass {
            id: gbufferPass
            layerMask: ContentLayer.Layer0 | ContentLayer.Layer1
            depthTexture: mainDepthStencilTexture
        }

        RenderOutputProvider {
            id: gbuffer0Provider
            textureSource: RenderOutputProvider.UserPassTexture
            renderPass: gbufferPass
            attachmentSelector: RenderOutputProvider.Attachment0
        }

        RenderOutputProvider {
            id: gbuffer1Provider
            textureSource: RenderOutputProvider.UserPassTexture
            renderPass: gbufferPass
            attachmentSelector: RenderOutputProvider.Attachment1
        }

        RenderOutputProvider {
            id: gbuffer2Provider
            textureSource: RenderOutputProvider.UserPassTexture
            renderPass: gbufferPass
            attachmentSelector: RenderOutputProvider.Attachment2
        }
        // ![GBufferPass usage]

        Model {
            id: donut
            layers: ContentLayer.Layer1
            y: 100
            z: 4
            geometry: TorusGeometry {
                radius: 100
                tubeRadius: 30
                segments: 64
                rings: 32
            }
            materials: [
                PrincipledMaterial {
                    baseColor: "#ffcc00"
                    metalness: 0.0
                    roughness: 0.5
                }
            ]

            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 8000
                loops: Animation.Infinite
                running: true
            }
        }

        Model {
            id: ground
            layers: ContentLayer.Layer0
            geometry: PlaneGeometry {
                width: 1000
                height: 1000
                plane: PlaneGeometry.XZ
            }
            materials: [
                PrincipledMaterial {
                    baseColor: "brown"
                    metalness: 0.0
                    roughness: 0.5
                }

            ]
        }

        // ![transparent cone]
        Model {
            id: cone
            layers: ContentLayer.Layer1
            source: "#Cone"
            y: 100
            materials: [
                PrincipledMaterial {
                    baseColor: Qt.rgba(0.0, 1.0, 0.0, 0.5)
                    alphaMode: PrincipledMaterial.Blend
                    metalness: 0.0
                    roughness: 0.5
                }
            ]
        }
        // ![transparent cone]

        // ![item2d node]
        Node {
            x: -200
            y: 100

            Item {
                anchors.centerIn: parent
                ColumnLayout {
                    Button {
                        text: "Click Me!"
                    }
                    Rectangle {
                        color: "blue"
                        implicitWidth: 50
                        implicitHeight: 50

                        NumberAnimation on rotation {
                            from: 0
                            to: 360
                            duration: 4000
                            loops: Animation.Infinite
                            running: true
                        }
                    }
                }
            }

            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 6000
                loops: Animation.Infinite
                running: true
            }
        }
        // ![item2d node]

        OrbitCameraController {
            origin: originNode
            camera: cameraNode
        }
    }

}
