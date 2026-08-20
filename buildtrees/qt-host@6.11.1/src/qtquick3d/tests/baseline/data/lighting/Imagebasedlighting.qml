// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

import "./materials" as Materials

Rectangle {
    id: imagebasedlighting
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0
        width: parent.width * 0.5
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
            lightProbe: layer_lightprobe
            probeExposure: 10
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 1000)
            clipFar: 5000
            fieldOfViewOrientation: PerspectiveCamera.Horizontal
        }

        Texture {
            id: layer_lightprobe
            source: "maps/OpenfootageNET_fieldairport-512.hdr"
            mappingMode: Texture.LightProbe
            tilingModeHorizontal: Texture.Repeat
        }

        Node {
            id: group
            position: Qt.vector3d(468.305, -129.677, 0)
            scale: Qt.vector3d(2, 2, 2)

            Model {
                id: cylinder
                position: Qt.vector3d(-448.399, 78.869, 0)
                source: "#Cylinder"
                
                

                Materials.Default_002 {
                    id: default_
                }
                materials: [default_]
            }

            Model {
                id: cone
                position: Qt.vector3d(-452.899, -147.545, 0)
                source: "#Cone"
                
                

                PrincipledMaterial {
                    id: default_001
                    metalness: 1
                }
                materials: [default_001]
            }

            Model {
                id: sphere
                position: Qt.vector3d(-441.352, 255.317, 0)
                source: "#Sphere"
                
                

                DefaultMaterial {
                    id: default_002
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_002]
            }

            Model {
                id: defaultOverride
                position: Qt.vector3d(-233.489, 252.353, 0)
                source: "#Sphere"
                
                

                DefaultMaterial {
                    id: default_003
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                    lightProbe: default_003_lightProbe

                    Texture {
                        id: default_003_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_003]
            }

            Model {
                id: referenceMaterialOverride
                position: Qt.vector3d(-235.717, 78.869, 0)
                source: "#Cylinder"
                
                

                Materials.Default_002 {
                    id: default_004
                    lightProbe: default_004_lightProbe

                    Texture {
                        id: default_004_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_004]
            }

            Model {
                id: customMaterialOverride
                position: Qt.vector3d(-245.686, -147.545, 0)
                source: "#Cone"
                
                

                PrincipledMaterial {
                    id: default_005
                    lightProbe: default_005_lightProbe
                    metalness: 1

                    Texture {
                        id: default_005_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_005]
            }

            Model {
                id: defaultMaterialSpecial
                position: Qt.vector3d(-18.5029, 255.317, 0)
                source: "#Sphere"
                
                

                DefaultMaterial {
                    id: default_006
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                    lightProbe: default_006_lightProbe

                    Texture {
                        id: default_006_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        scaleU: 20
                        scaleV: 0
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_006]
            }

            Model {
                id: referenceMaterialSpecial
                position: Qt.vector3d(-14.5576, 78.869, 0)
                source: "#Cylinder"
                
                

                Materials.Default_002 {
                    id: default_007
                    lightProbe: default_007_lightProbe

                    Texture {
                        id: default_007_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        scaleU: 20
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_007]
            }

            Model {
                id: customMaterialSpecial
                position: Qt.vector3d(-17.4744, -147.545, 0)
                source: "#Cone"
                
                

                PrincipledMaterial {
                    id: copper_001
                    lightProbe: copper_001_lightProbe
                    metalness: 1

                    Texture {
                        id: copper_001_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        scaleU: 20
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [copper_001]
            }
        }
    }

    View3D {
        id: oneLightProbe
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.5
        width: parent.width * 0.5
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
            lightProbe: oneLightProbe_lightprobe
            probeExposure: 10
        }

        PerspectiveCamera {
            id: camera_001
            position: Qt.vector3d(0, 0, 1000)
            clipFar: 5000
            fieldOfViewOrientation: PerspectiveCamera.Horizontal
        }

        Texture {
            id: oneLightProbe_lightprobe
            source: "maps/OpenfootageNET_fieldairport-512.hdr"
            mappingMode: Texture.LightProbe
            tilingModeHorizontal: Texture.Repeat
        }

        Node {
            id: group_001
            position: Qt.vector3d(468.305, -129.677, 0)
            scale: Qt.vector3d(2, 2, 2)

            Model {
                id: referenceMaterial
                position: Qt.vector3d(-448.399, 78.869, 0)
                source: "#Cylinder"
                
                

                Materials.Default_009 {
                    id: default_008
                }
                materials: [default_008]
            }

            Model {
                id: customMaterial
                position: Qt.vector3d(-452.899, -147.545, 0)
                source: "#Cone"
                
                

                PrincipledMaterial {
                    id: copper_002
                    metalness: 1
                }
                materials: [copper_002]
            }

            Model {
                id: defaultMaterial
                position: Qt.vector3d(-441.352, 255.317, 0)
                source: "#Sphere"
                
                

                DefaultMaterial {
                    id: default_009
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_009]
            }

            Model {
                id: defaultOverride_001
                position: Qt.vector3d(-233.489, 252.353, 0)
                source: "#Sphere"
                
                

                DefaultMaterial {
                    id: default_010
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                    lightProbe: default_010_lightProbe

                    Texture {
                        id: default_010_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_010]
            }

            Model {
                id: referenceMaterialOverride_001
                position: Qt.vector3d(-235.717, 78.869, 0)
                source: "#Cylinder"
                
                

                Materials.Default_009 {
                    id: default_011
                    lightProbe: default_011_lightProbe

                    Texture {
                        id: default_011_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_011]
            }

            Model {
                id: customMaterialOverride_001
                position: Qt.vector3d(-245.686, -147.545, 0)
                source: "#Cone"
                
                

                PrincipledMaterial {
                    id: copper_003
                    lightProbe: copper_003_lightProbe
                    metalness: 1

                    Texture {
                        id: copper_003_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [copper_003]
            }

            Model {
                id: defaultMaterialSpecial_001
                position: Qt.vector3d(-18.5029, 255.317, 0)
                source: "#Sphere"
                
                

                DefaultMaterial {
                    id: default_012
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                    lightProbe: default_012_lightProbe

                    Texture {
                        id: default_012_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        scaleU: 20
                        scaleV: 0
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_012]
            }

            Model {
                id: referenceMaterialSpecial_001
                position: Qt.vector3d(-14.5576, 78.869, 0)
                source: "#Cylinder"
                
                

                Materials.Default_009 {
                    id: default_013
                    lightProbe: default_013_lightProbe

                    Texture {
                        id: default_013_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        scaleU: 20
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [default_013]
            }

            Model {
                id: customMaterialSpecial_001
                position: Qt.vector3d(-17.4744, -147.545, 0)
                source: "#Cone"
                
                

                PrincipledMaterial {
                    id: copper_004
                    metalness: 1
                    lightProbe: copper_004_lightProbe

                    Texture {
                        id: copper_004_lightProbe
                        source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
                        scaleU: 20
                        mappingMode: Texture.LightProbe
                        tilingModeHorizontal: Texture.Repeat
                    }
                }
                materials: [copper_004]
            }
        }
    }
}
