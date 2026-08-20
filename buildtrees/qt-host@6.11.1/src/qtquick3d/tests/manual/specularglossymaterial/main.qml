// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    title: "SpecularGlossyMaterial Example"
    color: "#848895"

    View3D {
        anchors.fill: parent
        camera: camera

        // Rotate the light direction
        DirectionalLight {
            eulerRotation.y: -100
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 5000
                    to: 360
                    from: 0
                }
            }
        }

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            probeExposure: 1.5
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData {}
            }
            skyboxBlurAmount: 0.4
        }

        Node {
            scale: Qt.vector3d(100, 100, 100)
            Model {
                id: waterBottle_MR
                x: -0.08
                rotation: Qt.quaternion(0, 0, 1, 0)
                source: "meshes/waterBottle_MR.mesh"

                PrincipledMaterial {
                    id: bottleMat_MR_material
                    baseColorMap: Texture {
                        source: "maps/WaterBottle_baseColor.jpg"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    opacityChannel: Material.A

                    Texture {
                        id: metalRoughnessTexture
                        source: "maps/WaterBottle_roughnessMetallic.jpg"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    metalnessMap: metalRoughnessTexture
                    metalnessChannel: Material.B
                    roughnessMap: metalRoughnessTexture
                    roughnessChannel: Material.G
                    metalness: 1
                    roughness: 1
                    normalMap: Texture {
                        source: "maps/WaterBottle_normal.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    occlusionMap: Texture {
                        source: "maps/WaterBottle_occlusion.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    occlusionChannel: Material.R
                    emissiveMap: Texture {
                        source: "maps/WaterBottle_emissive.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    emissiveFactor: Qt.vector3d(1, 1, 1)
                    alphaMode: PrincipledMaterial.Opaque
                }
                materials: [
                    bottleMat_MR_material
                ]
            }
            Model {
                id: waterBottle_SpecGloss
                x: 0.08
                rotation: Qt.quaternion(0, 0, 1, 0)
                source: "meshes/waterBottle_SpecGloss.mesh"

                SpecularGlossyMaterial {
                    id: bottleMat_SpecGloss_material
                    albedoMap: Texture {
                        source: "maps/WaterBottle_diffuse.jpg"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }

                    normalMap: Texture {
                        source: "maps/WaterBottle_normal.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    occlusionMap: Texture {
                        source: "maps/WaterBottle_occlusion.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    occlusionChannel: Material.R
                    emissiveMap: Texture {
                        source: "maps/WaterBottle_emissive.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    emissiveFactor: Qt.vector3d(1, 1, 1)
                    alphaMode: PrincipledMaterial.Opaque

                    Texture {
                        id: specularGlossyTexture
                        source: "maps/WaterBottle_specularGlossiness.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }

                    specularMap: specularGlossyTexture
                    glossiness: 1.0
                    glossinessMap: specularGlossyTexture
                    glossinessChannel: Material.A
                }
                materials: [
                    bottleMat_SpecGloss_material
                ]
            }
        }

        Node {
            id: originNode
            PerspectiveCamera {
                id: cameraNode
                position: Qt.vector3d(0, 0, 30)
            }
        }

        OrbitCameraController {
            origin: originNode
            camera: cameraNode
        }
    }
}
