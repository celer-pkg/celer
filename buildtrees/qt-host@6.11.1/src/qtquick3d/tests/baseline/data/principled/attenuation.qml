// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 900
    height: 450
    color: "lightgray"

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                source: "../shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
            }
        }

        PerspectiveCamera {
            id: camera1
            z: 15

        }

        Node {
            id: rOOT
            Model {
                id: r2_Block_1_0
                y: 3
                source: "../shared/models/attenuation/r2_Block_1_0.mesh"

                PrincipledMaterial {
                    id: r2_and_R4_ThicknessFac_1_0_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 1
                    attenuationDistance: 1
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r2_and_R4_ThicknessFac_1_0_material
                ]
            }
            Model {
                id: r2_Block_1_5
                x: 2.5
                y: 3
                source: "../shared/models/attenuation/r2_Block_1_5.mesh"

                PrincipledMaterial {
                    id: r2_ThicknessFac_1_5_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 1.5
                    attenuationDistance: 1
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r2_ThicknessFac_1_5_material
                ]
            }
            Model {
                id: r2_Block_2_0
                x: 6
                y: 3
                source: "../shared/models/attenuation/r2_Block_2_0.mesh"

                PrincipledMaterial {
                    id: r2_ThicknessFac_2_0_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 2
                    attenuationDistance: 1
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r2_ThicknessFac_2_0_material
                ]
            }
            Model {
                id: r2_Block_0_50
                x: -2
                y: 3
                source: "../shared/models/attenuation/r2_Block_0_50.mesh"

                PrincipledMaterial {
                    id: r2_ThicknessFac_0_50_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 0.5
                    attenuationDistance: 1
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r2_ThicknessFac_0_50_material
                ]
            }
            Model {
                id: r2_Block_0_25
                x: -3.5
                y: 3
                source: "../shared/models/attenuation/r2_Block_0_25.mesh"

                PrincipledMaterial {
                    id: r2_ThicknessFac_0_25_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 0.25
                    attenuationDistance: 1
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r2_ThicknessFac_0_25_material
                ]
            }
            Model {
                id: labels
                source: "../shared/models/attenuation/labels.mesh"

                PrincipledMaterial {
                    id: labelMaterial_material
                    baseColorMap: Texture {
                        source: "../shared/maps/AttenuationLabels.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    opacityChannel: Material.A
                    roughness: 0.8
                    alphaMode: PrincipledMaterial.Opaque
                }
                materials: [
                    labelMaterial_material
                ]
            }
            Model {
                id: r4_Block_1_0
                y: -3
                source: "../shared/models/attenuation/r4_Block_1_0.mesh"
                materials: [
                    r2_and_R4_ThicknessFac_1_0_material
                ]
            }
            Model {
                id: r4_Block_1_5
                x: 2.5
                y: -3
                scale.x: 1.5
                scale.y: 1.5
                scale.z: 1.5
                source: "../shared/models/attenuation/r4_Block_1_5.mesh"
                materials: [
                    r2_and_R4_ThicknessFac_1_0_material
                ]
            }
            Model {
                id: r4_Block_2_0
                x: 6
                y: -3
                scale.x: 2
                scale.y: 2
                scale.z: 2
                source: "../shared/models/attenuation/r4_Block_2_0.mesh"
                materials: [
                    r2_and_R4_ThicknessFac_1_0_material
                ]
            }
            Model {
                id: r4_Block_0_50
                x: -2
                y: -3
                scale.x: 0.5
                scale.y: 0.5
                scale.z: 0.5
                source: "../shared/models/attenuation/r4_Block_0_50.mesh"
                materials: [
                    r2_and_R4_ThicknessFac_1_0_material
                ]
            }
            Model {
                id: r4_Block_0_25
                x: -3.5
                y: -3
                scale.x: 0.25
                scale.y: 0.25
                scale.z: 0.25
                source: "../shared/models/attenuation/r4_Block_0_25.mesh"
                materials: [
                    r2_and_R4_ThicknessFac_1_0_material
                ]
            }
            Model {
                id: r3_Block_Row
                x: -3.5
                source: "../shared/models/attenuation/r3_Block_Row.mesh"

                PrincipledMaterial {
                    id: r3_ThicknessTex_Mat_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 2
                    thicknessMap: Texture {
                        source: "../shared/maps/ThicknessTexture.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    attenuationDistance: 1
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r3_ThicknessTex_Mat_material
                ]
            }
            Model {
                id: r5_Block_1_0
                y: -6
                source: "../shared/models/attenuation/r5_Block_1_0.mesh"

                PrincipledMaterial {
                    id: r5_Attenuation_1_0_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 1
                    attenuationDistance: 1
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r5_Attenuation_1_0_material
                ]
            }
            Model {
                id: r5_Block_1_5
                x: 2.5
                y: -6
                source: "../shared/models/attenuation/r5_Block_1_5.mesh"

                PrincipledMaterial {
                    id: r5_Attenuation_1_5_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 1
                    attenuationDistance: 0.666667
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r5_Attenuation_1_5_material
                ]
            }
            Model {
                id: r5_Block_2_0
                x: 6
                y: -6
                source: "../shared/models/attenuation/r5_Block_2_0.mesh"

                PrincipledMaterial {
                    id: r5_Attenuation_2_0_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 1
                    attenuationDistance: 0.5
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r5_Attenuation_2_0_material
                ]
            }
            Model {
                id: r5_Block_0_50
                x: -2
                y: -6
                source: "../shared/models/attenuation/r5_Block_0_50.mesh"

                PrincipledMaterial {
                    id: r5_Attenuation_0_50_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 1
                    attenuationDistance: 2
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r5_Attenuation_0_50_material
                ]
            }
            Model {
                id: r5_Block_0_25
                x: -3.5
                y: -6
                source: "../shared/models/attenuation/r5_Block_0_25.mesh"

                PrincipledMaterial {
                    id: r5_Attenuation_0_25_material
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                    thicknessFactor: 1
                    attenuationDistance: 4
                    attenuationColor: "#ff1a80e6"
                }
                materials: [
                    r5_Attenuation_0_25_material
                ]
            }
            Model {
                id: meterGrid
                z: -2
                source: "../shared/models/attenuation/meterGrid.mesh"

                PrincipledMaterial {
                    id: flatBackdrop_material
                    baseColorMap: Texture {
                        source: "../shared/maps/PlainGrid.png"
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    opacityChannel: Material.A
                    roughness: 0.5
                    alphaMode: PrincipledMaterial.Opaque
                }
                materials: [
                    flatBackdrop_material
                ]
            }
            Model {
                id: r1_Sample_1_0
                y: 6
                source: "../shared/models/attenuation/r1_Sample_1_0.mesh"

                PrincipledMaterial {
                    id: r1_Sample_1_0_material
                    baseColor: "#ff1a80e6"
                    cullMode: Material.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                }
                materials: [
                    r1_Sample_1_0_material
                ]
            }
            Model {
                id: r1_Sample_1_5
                x: 2.5
                y: 6
                source: "../shared/models/attenuation/r1_Sample_1_5.mesh"

                PrincipledMaterial {
                    id: r1_Sample_1_5_material
                    baseColor: "#ff085ada"
                    cullMode: Material.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                }
                materials: [
                    r1_Sample_1_5_material
                ]
            }
            Model {
                id: r1_Sample_2_0
                x: 6
                y: 6
                source: "../shared/models/attenuation/r1_Sample_2_0.mesh"

                PrincipledMaterial {
                    id: r1_Sample_2_0_material
                    baseColor: "#ff0340cf"
                    cullMode: Material.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                }
                materials: [
                    r1_Sample_2_0_material
                ]
            }
            Model {
                id: r1_Sample_0_50
                x: -2
                y: 6
                source: "../shared/models/attenuation/r1_Sample_0_50.mesh"

                PrincipledMaterial {
                    id: r1_Sample_0_50_material
                    baseColor: "#ff51b4f2"
                    cullMode: Material.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                }
                materials: [
                    r1_Sample_0_50_material
                ]
            }
            Model {
                id: r1_Sample_0_25
                x: -3.5
                y: 6
                source: "../shared/models/attenuation/r1_Sample_0_25.mesh"

                PrincipledMaterial {
                    id: r1_Sample_0_25_material
                    baseColor: "#ff8fd6f8"
                    cullMode: Material.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                    transmissionFactor: 1
                }
                materials: [
                    r1_Sample_0_25_material
                ]
            }
        }
    }
}
