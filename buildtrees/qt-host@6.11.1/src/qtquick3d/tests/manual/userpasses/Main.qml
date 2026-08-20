// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

    SplitView {
        id: editorViews
        anchors.fill: parent
        orientation: Qt.Horizontal

        Page {
            id: settingsPage
            SplitView.preferredWidth: editorViews.width * 0.2
            title: "Settings"
            header: ToolBar {
                width: parent.width
                Label {
                    width: parent.width
                    text: settingsPage.title
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 20
                }
            }
            ScrollView {
                anchors.fill: parent
                contentHeight: editableSections.implicitHeight

                ColumnLayout {
                    id: editableSections
                    width: parent.width
                    spacing: 0

                    CheckBox {
                        id: enableNormalMapPassCheckbox
                        text: "Enable Normal Map Pass"
                        checked: view3D.enableNormalMapPass
                        onCheckedChanged: {
                            view3D.enableNormalMapPass = checked
                        }
                    }
                }
            }
        }

        View3D {
            id: view3D
            property bool enableNormalMapPass: true
            SplitView.preferredWidth: editorViews.width * 0.8
            renderOverrides: View3D.DisableInternalPasses
            environment: SceneEnvironment {
                lightProbe: Texture {
                    textureData: ProceduralSkyTextureData {

                    }

                }
                backgroundMode: SceneEnvironment.SkyBox
            }
            // camera: cameraNode

            SimpleQuadRenderer {
                texture: Texture {
                    textureProvider: mainColorPassProvider
                }
            }

            // Render Path:
            // Opaque Items (GBuffer) -
            // Opaque Items (lighting pass)-
            // Skybox
            // Item2Ds
            // Transparent Items [Original Mode]


            RenderPassTexture {
                id: mainColorTexture
                format: RenderPassTexture.RGBA16F
            }



            RenderPassTexture {
                id: mainDepthStencilTexture
                format: RenderPassTexture.Depth24Stencil8
            }

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
                //ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0);
            }

            // Ground
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


            Model {
                id: deferredLightingQuad
                layers: ContentLayer.Layer13
                geometry: PlaneGeometry {
                    // geometry doesn't matter, just need 4 verts
                    plane: PlaneGeometry.XY
                }
                castsShadows: false
                receivesShadows: false
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

            RenderPass {
                id: mainColorPass
                clearColor: "black"
                // Don't clear the depth texture
                renderTargetFlags: RenderPass.RenderTargetFlags.PreserveDepthStencilContents

                commands: [
                    ColorAttachment {
                        target: mainColorTexture
                    },
                    DepthTextureAttachment {
                        target: mainDepthStencilTexture
                    },
                    RenderablesFilter {
                        id: filter
                        renderableTypes: RenderablesFilter.None // Don't render anything in this pass, only SubRenderPasses
                    },
                    SubRenderPass {
                        renderPass: RenderPass {
                            id: deferredLightingPass

                            readonly property Texture gbuffer0: Texture { textureProvider: gbuffer0Provider }
                            readonly property Texture gbuffer1: Texture { textureProvider: gbuffer1Provider }
                            readonly property Texture gbuffer2: Texture { textureProvider: gbuffer2Provider }

                            materialMode: RenderPass.OriginalMaterial

                            commands: [
                                PipelineStateOverride {
                                    // We shouldn't write the quad into the depth buffer
                                    depthWriteEnabled: false
                                    depthTestEnabled: false
                                },
                                RenderablesFilter { layerMask: ContentLayer.Layer13 }
                            ]
                        }
                    },
                    SubRenderPass {
                        renderPass: RenderPass {
                            id: skyboxPass
                            passMode: RenderPass.SkyboxPass
                            commands: [
                                PipelineStateOverride {
                                    depthTestEnabled: true
                                    depthWriteEnabled: false
                                }
                            ]
                        }
                    },
                    SubRenderPass {
                        renderPass: RenderPass {
                            id: item2DPass
                            passMode: RenderPass.Item2DPass
                        }
                    },
                    SubRenderPass {
                        renderPass: RenderPass {
                            id: transparentItemPass
                            materialMode: RenderPass.OriginalMaterial

                            commands: [
                                RenderablesFilter {
                                    renderableTypes: RenderablesFilter.Transparent
                                    layerMask: ContentLayer.Layer0 | ContentLayer.Layer1 | ContentLayer.Layer2 | ContentLayer.Layer4
                                },
                                PipelineStateOverride {
                                    // Enable blending for transparent objects
                                    blendEnabled: true
                                    depthTestEnabled: true
                                }

                            ]
                        }
                    }
                ]
            }

            RenderOutputProvider {
                id: mainColorPassProvider
                textureSource: RenderOutputProvider.UserPassTexture
                renderPass: mainColorPass
                attachmentSelector: RenderOutputProvider.Attachment0
            }



            GBufferPass {
                id: gbufferPass
                layerMask: ContentLayer.Layer0 | ContentLayer.Layer1 | ContentLayer.Layer2 | ContentLayer.Layer4
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

            // Random Donut

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
                id: cone
                layers: ContentLayer.Layer1
                y: 100
                x: 0
                source: "#Cone"
                // geometry: ConeGeometry {
                //     topRadius: 0
                //     bottomRadius: 50
                //     // slices: 64
                // }

                materials: [
                    PrincipledMaterial {
                        baseColor: Qt.rgba(0.0, 1.0, 0.0, 0.5)
                        alphaMode: PrincipledMaterial.Blend
                        metalness: 0.0
                        roughness: 0.5
                    }
                ]
            }


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
                    duration: 2000
                    easing.type: Easing.InOutQuad
                    loops: Animation.Infinite
                    running: true
                }
            }

            OrbitCameraController {
                origin: originNode
                camera: cameraNode
            }
        }
    }
}
