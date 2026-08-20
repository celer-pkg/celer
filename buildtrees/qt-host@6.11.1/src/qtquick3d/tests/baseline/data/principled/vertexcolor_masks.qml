// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 540
    height: 500
    color: "lightgray"

    View3D {
        anchors.fill: parent
        camera: camera
        renderMode: View3D.Offscreen

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
            lightProbe: Texture {
                source: "../shared/maps/TestEnvironment-512.hdr"
            }
        }

        OrthographicCamera {
            id: camera
            z: 500
        }

        Texture {
            id: tex_normal
            source: "../shared/maps/wrinkles_normal.jpg"
        }

        Texture {
            id: tex_stripe
            source: "../shared/maps/texture14184.jpg"
        }

        Model {
            source: "#Rectangle"
            materials: [ DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                    diffuseMap: Texture {
                        source: "../shared/maps/checkers1.png"
                        tilingModeHorizontal: Texture.Repeat
                        tilingModeVertical: Texture.Repeat
                        scaleU: 100
                        scaleV: 100
                    }
                } ]
            z: -500
            scale: Qt.vector3d(10, 10, 1)
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-67, 200, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 1.0
                    roughness: 1.0
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    vertexColorRedMask: PrincipledMaterial.MetalnessMask
                } ]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-67, 100, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 1.0
                    roughness: 1.0
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    vertexColorRedMask: PrincipledMaterial.RoughnessMask
                } ]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-67, 0, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 1.0
                    roughness: 1.0
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    clearcoatAmount: 1.0
                    vertexColorGreenMask: PrincipledMaterial.ClearcoatAmountMask
                } ]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-67, -100, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 1.0
                    roughness: 1.0
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    clearcoatAmount: 1.0
                    clearcoatRoughnessAmount: 1.0
                    vertexColorRedMask: PrincipledMaterial.ClearcoatRoughnessAmountMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-67, -200, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 1.0
                    roughness: 1.0
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    clearcoatAmount: 1.0
                    clearcoatNormalMap: tex_normal
                    vertexColorGreenMask: PrincipledMaterial.ClearcoatNormalStrengthMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-201, 200.0, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 0.5
                    roughness: 0.5
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    vertexColorAlphaMask: PrincipledMaterial.SpecularAmountMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-201, 100.0, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 1.0
                    roughness: 0.4
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    normalMap: tex_normal
                    vertexColorGreenMask: PrincipledMaterial.NormalStrengthMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-201, 0, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 1.0
                    roughness: 0.4
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    heightAmount: 1.0
                    heightMap: tex_stripe
                    normalMap: tex_normal
                    vertexColorAlphaMask: PrincipledMaterial.HeightAmountMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-201, -100, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 1.0
                    roughness: 0.4
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    occlusionMap: tex_stripe
                    vertexColorGreenMask: PrincipledMaterial.OcclusionAmountMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(-201, -200, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 0.0
                    roughness: 0.0
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    transmissionFactor: 0.5
                    vertexColorRedMask: PrincipledMaterial.TransmissionFactorMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(66, 200.0, 0)
            materials: [ PrincipledMaterial {
                    baseColor: "white"
                    metalness: 0.0
                    roughness: 0.0
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    transmissionFactor: 0.5
                    thicknessFactor: 0.5
                    vertexColorRedMask: PrincipledMaterial.ThicknessFactorMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(66, 100.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    transmissionFactor: 0.5
                    thicknessFactor: 0.5
                    vertexColorRedMask: SpecularGlossyMaterial.ThicknessFactorMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(66, 0.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    transmissionFactor: 0.5
                    vertexColorRedMask: SpecularGlossyMaterial.TransmissionFactorMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(66, -100.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    occlusionMap: tex_stripe
                    vertexColorGreenMask: SpecularGlossyMaterial.OcclusionAmountMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(66, -200.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    glossiness: 1.0
                    vertexColorRedMask: SpecularGlossyMaterial.GlossinessMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(201, 200.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    normalMap: tex_normal
                    vertexColorBlueMask: SpecularGlossyMaterial.NormalStrengthMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(201, 100.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    heightAmount: 1.0
                    heightMap: tex_stripe
                    normalMap: tex_normal
                    vertexColorAlphaMask: SpecularGlossyMaterial.HeightAmountMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(201, 0.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    clearcoatAmount: 1.0
                    clearcoatNormalMap: tex_normal
                    vertexColorBlueMask: SpecularGlossyMaterial.ClearcoatNormalStrengthMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(201, -100.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    clearcoatAmount: 1.0
                    clearcoatRoughnessAmount: 1.0
                    vertexColorRedMask: SpecularGlossyMaterial.ClearcoatRoughnessAmountMask
                }]
        }

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"
            scale: Qt.vector3d(50.0, 50.0, 50.0)
            position: Qt.vector3d(201, -200.0, 0)
            materials: [ SpecularGlossyMaterial {
                    albedoColor: "white"
                    vertexColorsEnabled: false
                    vertexColorsMaskEnabled: true
                    clearcoatAmount: 1.0
                    vertexColorGreenMask: SpecularGlossyMaterial.ClearcoatAmountMask
                }]
        }
    }
}
