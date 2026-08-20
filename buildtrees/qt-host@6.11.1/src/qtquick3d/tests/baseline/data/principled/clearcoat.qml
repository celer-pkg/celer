// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 400
    height: 500
    color: "lightgray"

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            lightProbe: Texture {
                source: "../shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
            }
        }

        PerspectiveCamera {
            id: camera1
            z: 300
        }

        DirectionalLight {

        }

        PointLight {
            z: 100
        }

        Node {
            scale: Qt.vector3d(25, 25, 25)
            Node {
                id: r0_SimpleCoatTest
                y: 5.25
                Model {
                    id: baseLayerSample
                    x: -2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: simple_Base_material
                        baseColor: "#ff800503"
                        roughness: 0.44
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        simple_Base_material
                    ]
                }
                Model {
                    id: clearCoatSample
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: simple_Coated_material
                        baseColor: "#ff800503"
                        roughness: 0.44
                        alphaMode: PrincipledMaterial.Opaque
                        clearcoatAmount: 1
                        clearcoatRoughnessAmount: 0.03
                    }
                    materials: [
                        simple_Coated_material
                    ]
                }
                Model {
                    id: coatOnlySample
                    x: 2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: simple_Coating_material
                        baseColor: "#ff000000"
                        roughness: 0.03
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        simple_Coating_material
                    ]
                }
            }
            Node {
                id: r1_PartialCoatTest
                y: 3.15
                Model {
                    id: r1_BaseLayerSample
                    x: -2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: partial_Base_material
                        baseColor: "#ff03061c"
                        roughness: 0.44
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        partial_Base_material
                    ]
                }
                Model {
                    id: r1_ClearCoatSample
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: partial_Coated_material
                        baseColor: "#ff03061c"
                        roughness: 0.44
                        alphaMode: PrincipledMaterial.Opaque
                        clearcoatAmount: 1
                        clearcoatRoughnessAmount: 0.03
                        clearcoatMap: Texture {
                            source: "../shared/maps/PartialCoating.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                    materials: [
                        partial_Coated_material
                    ]
                }
                Model {
                    id: r1_CoatOnlySample
                    x: 2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: partial_Coating_material
                        baseColorMap: Texture {
                            source: "../shared/maps/PartialCoating_Alpha.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        opacityChannel: Material.A
                        roughness: 0.03
                        alphaMode: PrincipledMaterial.Blend
                    }
                    materials: [
                        partial_Coating_material
                    ]
                }
            }
            Node {
                id: r2_RoughnessVariations
                y: 1.05
                Model {
                    id: r2_BaseLayerSample
                    x: -2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: roughVariations_Base_material
                        baseColor: "#ff03061c"
                        roughness: 0.6
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        roughVariations_Base_material
                    ]
                }
                Model {
                    id: r2_ClearCoatSample
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: roughVariations_Coated_material
                        baseColor: "#ff03061c"
                        roughness: 0.6
                        alphaMode: PrincipledMaterial.Opaque
                        clearcoatAmount: 1
                        clearcoatRoughnessAmount: 1
                        clearcoatRoughnessMap: Texture {
                            source: "../shared/maps/RoughnessStripes.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                    materials: [
                        roughVariations_Coated_material
                    ]
                }
                Model {
                    id: r2_CoatOnlySample
                    x: 2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: roughVariations_Coating_material
                        baseColor: "#ff000000"
                        metalnessMap: Texture {
                            source: "../shared/maps/RoughnessStripes.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        metalnessChannel: Material.B
                        roughnessMap: Texture {
                            source: "../shared/maps/RoughnessStripes.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        roughnessChannel: Material.G
                        roughness: 1
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        roughVariations_Coating_material
                    ]
                }
            }
            Node {
                id: r3_BaseNormals
                y: -1.05
                Model {
                    id: r3_BaseLayerSample
                    x: -2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: baseNorm_Base_material
                        baseColor: "#ff03061c"
                        roughness: 0.44
                        normalMap: Texture {
                            source: "../shared/maps/RibsNormal.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        baseNorm_Base_material
                    ]
                }
                Model {
                    id: r3_ClearCoatSample
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: baseNorm_Coated_material
                        baseColor: "#ff03061c"
                        roughness: 0.44
                        normalMap: Texture {
                            source: "../shared/maps/RibsNormal.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        alphaMode: PrincipledMaterial.Opaque
                        clearcoatAmount: 1
                        clearcoatRoughnessAmount: 0.03
                    }
                    materials: [
                        baseNorm_Coated_material
                    ]
                }
                Model {
                    id: r3_CoatOnlySample
                    x: 2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: baseNorm_Coating_material
                        baseColor: "#ff000000"
                        roughness: 0.03
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        baseNorm_Coating_material
                    ]
                }
            }
            Node {
                id: r4_CoatNormals
                y: -5.25
                Model {
                    id: r4_BaseLayerSample
                    x: -2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: coatNorm_Base_material
                        baseColor: "#ff03061c"
                        roughness: 0.44
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        coatNorm_Base_material
                    ]
                }
                Model {
                    id: r4_ClearCoatSample
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: coatNorm_Coated_material
                        baseColor: "#ff03061c"
                        roughness: 0.44
                        alphaMode: PrincipledMaterial.Opaque
                        clearcoatAmount: 1
                        clearcoatRoughnessAmount: 0.03
                        clearcoatNormalMap: Texture {
                            source: "../shared/maps/PlasticWrap_normals.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                    materials: [
                        coatNorm_Coated_material
                    ]
                }
                Model {
                    id: r4_CoatOnlySample
                    x: 2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: coatNorm_Coating_material
                        baseColor: "#ff000000"
                        roughness: 0.03
                        normalMap: Texture {
                            source: "../shared/maps/PlasticWrap_normals.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        coatNorm_Coating_material
                    ]
                }
            }
            Node {
                id: r5_SharedNormals
                y: -3.15
                Model {
                    id: r5_BaseLayerSample
                    x: -2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: sharedNorm_Base_material
                        baseColor: "#ff03061c"
                        roughness: 0.44
                        normalMap: Texture {
                            source: "../shared/maps/RibsNormal.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        sharedNorm_Base_material
                    ]
                }
                Model {
                    id: r5_ClearCoatSample
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: sharedNorm_Coated_material
                        baseColor: "#ff03061c"
                        roughness: 0.44
                        normalMap: Texture {
                            source: "../shared/maps/RibsNormal.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        alphaMode: PrincipledMaterial.Opaque
                        clearcoatAmount: 1
                        clearcoatRoughnessAmount: 0.03
                        clearcoatNormalMap: Texture {
                            source: "../shared/maps/RibsNormal.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                    materials: [
                        sharedNorm_Coated_material
                    ]
                }
                Model {
                    id: r5_CoatOnlySample
                    x: 2.1
                    source: "../shared/models/baseLayerSample.mesh"

                    PrincipledMaterial {
                        id: sharedNorm_Coating_material
                        baseColor: "#ff000000"
                        roughness: 0.03
                        normalMap: Texture {
                            source: "../shared/maps/RibsNormal.png"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        sharedNorm_Coating_material
                    ]
                }
            }
        }
    }
}
