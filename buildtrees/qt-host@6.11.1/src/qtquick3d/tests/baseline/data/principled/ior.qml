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
            probeExposure: 3.0
        }

        PerspectiveCamera {
            id: camera1
            z: 0.75
            clipNear: 0.01
            clipFar: 100
        }

        Node {
            id: rOOT
            Model {
                id: labels
                x: -0.55
                source: "../shared/models/ior/labels.mesh"

                PrincipledMaterial {
                    id: labelMat_material
                    baseColorMap: Texture {
                        source: "../shared/maps/IOR_Labels.png"
                        minFilter: Texture.Nearest
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    opacityChannel: Material.A
                    roughness: 0.8
                    alphaMode: PrincipledMaterial.Opaque
                }
                materials: [
                    labelMat_material
                ]
            }
            Node {
                id: sampleRoot
                x: 0.2
                Model {
                    id: iOR_1_0
                    y: -0.22
                    source: "../shared/models/ior/opaque.mesh"

                    PrincipledMaterial {
                        id: mat_IOR_1_0_material
                        baseColor: "#ff80b3e6"
                        metalnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        metalnessChannel: Material.B
                        roughnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        roughnessChannel: Material.G
                        metalness: 1
                        roughness: 1
                        alphaMode: PrincipledMaterial.Opaque
                        transmissionFactor: 1
                        thicknessFactor: 0.005
                        indexOfRefraction: 1
                    }
                    materials: [
                        mat_IOR_1_0_material
                    ]
                }
                Model {
                    id: iOR_1_33
                    y: -0.11
                    source: "../shared/models/ior/opaque.mesh"

                    PrincipledMaterial {
                        id: mat_IOR_1_33_material
                        baseColor: "#ff80b3e6"
                        metalnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        metalnessChannel: Material.B
                        roughnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        roughnessChannel: Material.G
                        metalness: 1
                        roughness: 1
                        alphaMode: PrincipledMaterial.Opaque
                        transmissionFactor: 1
                        thicknessFactor: 0.005
                        indexOfRefraction: 1.33
                    }
                    materials: [
                        mat_IOR_1_33_material
                    ]
                }
                Model {
                    id: iOR_1_50
                    source: "../shared/models/ior/opaque.mesh"

                    PrincipledMaterial {
                        id: mat_IOR_1_50_material
                        baseColor: "#ff80b3e6"
                        metalnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        metalnessChannel: Material.B
                        roughnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        roughnessChannel: Material.G
                        metalness: 1
                        roughness: 1
                        alphaMode: PrincipledMaterial.Opaque
                        transmissionFactor: 1
                        thicknessFactor: 0.005
                    }
                    materials: [
                        mat_IOR_1_50_material
                    ]
                }
                Model {
                    id: iOR_1_76
                    y: 0.11
                    source: "../shared/models/ior/opaque.mesh"

                    PrincipledMaterial {
                        id: mat_IOR_1_76_material
                        baseColor: "#ff80b3e6"
                        metalnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        metalnessChannel: Material.B
                        roughnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        roughnessChannel: Material.G
                        metalness: 1
                        roughness: 1
                        alphaMode: PrincipledMaterial.Opaque
                        transmissionFactor: 1
                        thicknessFactor: 0.005
                        indexOfRefraction: 1.76
                    }
                    materials: [
                        mat_IOR_1_76_material
                    ]
                }
                Model {
                    id: iOR_2_42
                    y: 0.22
                    source: "../shared/models/ior/opaque.mesh"

                    PrincipledMaterial {
                        id: mat_IOR_2_42_material
                        baseColor: "#ff80b3e6"
                        metalnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        metalnessChannel: Material.B
                        roughnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        roughnessChannel: Material.G
                        metalness: 1
                        roughness: 1
                        alphaMode: PrincipledMaterial.Opaque
                        transmissionFactor: 1
                        thicknessFactor: 0.005
                        indexOfRefraction: 2.42
                    }
                    materials: [
                        mat_IOR_2_42_material
                    ]
                }
                Model {
                    id: opaque
                    y: -0.33
                    source: "../shared/models/ior/opaque.mesh"

                    PrincipledMaterial {
                        id: mat_Opaque_material
                        baseColor: "#ffcccccc"
                        metalnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid-1.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        metalnessChannel: Material.B
                        roughnessMap: Texture {
                            source: "../shared/maps/RoughnessGrid-1.png"
                            minFilter: Texture.Nearest
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                        roughnessChannel: Material.G
                        metalness: 1
                        roughness: 1
                        alphaMode: PrincipledMaterial.Opaque
                    }
                    materials: [
                        mat_Opaque_material
                    ]
                }
            }
            Model {
                id: flat_Backdrop
                z: 0.1
                source: "../shared/models/ior/flat_Backdrop.mesh"

                PrincipledMaterial {
                    id: flatBackdrop_material
                    baseColorMap: Texture {
                        source: "../shared/maps/GridWithDetails.png"
                        minFilter: Texture.Nearest
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
                id: smoothRoughLabels
                source: "../shared/models/ior/smoothRoughLabels.mesh"

                PrincipledMaterial {
                    id: smoothVsRough_material
                    baseColorMap: Texture {
                        source: "../shared/maps/SmoothVsRough.png"
                        minFilter: Texture.Nearest
                        generateMipmaps: true
                        mipFilter: Texture.Linear
                    }
                    opacityChannel: Material.A
                    roughness: 0.8
                    cullMode: Material.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                }
                materials: [
                    smoothVsRough_material
                ]
            }
        }
    }
}
