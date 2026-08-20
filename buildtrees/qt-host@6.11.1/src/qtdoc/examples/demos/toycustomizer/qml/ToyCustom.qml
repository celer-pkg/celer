// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root

    property var currentElement
    property alias accessoryModel: matLib.accessoryModel

    scale: Qt.vector3d(125, 125, 125)

    Skin {
        id: skin
        joints: [
            universalRigged,
            universalRiggedPelvis,
            universalRiggedLThigh,
            universalRiggedLCalf,
            universalRiggedLFoot,
            universalRiggedRThigh,
            universalRiggedRCalf,
            universalRiggedRFoot,
            universalRiggedSpine1,
            universalRiggedSpine2,
            universalRiggedSpine3,
            universalRiggedRibcage,
            universalRiggedLCollarbone,
            universalRiggedLUpperarm,
            universalRiggedLForearm,
            universalRiggedLPalm,
            universalRiggedRCollarbone,
            universalRiggedRUpperarm,
            universalRiggedRForearm,
            universalRiggedRPalm,
            universalRiggedNeck,
            universalRiggedHead,
            universalRiggedEar_L,
            universalRiggedEar_R,
            universalRiggedTail,
            universalRiggedLPlatform,
            universalRiggedRPlatform
        ]
        inverseBindPoses: [
            Qt.matrix4x4(0.0521354, 0.997101, -0.0554131, 0.00566421, -0.0998558, 0.0604155, 0.993166, -0.0113128, 0.993635, -0.0462458, 0.102716, -0.000159482, 0, 0, 0, 1),
            Qt.matrix4x4(0.0524837, 0.996885, -0.0588782, -0.129069, -0.0996906, 0.0638952, 0.992965, -0.00123965, 0.993634, -0.0462449, 0.102733, -0.00015946, 0, 0, 0, 1),
            Qt.matrix4x4(-0.0148187, -0.999795, 0.0138209, 0.11472, -0.101994, 0.0152618, 0.994668, -0.00236435, -0.994675, 0.0133301, -0.102199, 0.0683189, 0, 0, 0, 1),
            Qt.matrix4x4(-0.0295033, -0.961289, -0.273957, 0.0573166, -0.139327, -0.267446, 0.953447, 0.0161199, -0.989807, 0.0662993, -0.126043, 0.0652713, 0, 0, 0, 1),
            Qt.matrix4x4(-0.109644, -0.181131, 0.977328, 0.0139247, 0.0265916, 0.982369, 0.185049, -0.00465917, -0.993615, 0.0462783, -0.102895, 0.0657461, 0, 0, 0, 1),
            Qt.matrix4x4(-0.0802324, -0.996752, 0.00698451, 0.114732, -0.102266, 0.0152015, 0.99464, -0.00235578, -0.991516, 0.0790881, -0.103153, -0.0679977, 0, 0, 0, 1),
            Qt.matrix4x4(-0.00386155, -0.962573, -0.270995, 0.0573048, -0.0834417, -0.269742, 0.95931, 0.0160968, -0.996506, 0.0263168, -0.0792772, -0.0649614, 0, 0, 0, 1),
            Qt.matrix4x4(-0.109275, -0.181181, 0.977359, 0.0139144, 0.026588, 0.982364, 0.185081, -0.00465573, -0.993656, 0.0462107, -0.10253, -0.0654343, 0, 0, 0, 1),
            Qt.matrix4x4(0.0233255, 0.976558, 0.213989, -0.130806, -0.110221, -0.21023, 0.971419, 0.0362973, 0.993634, -0.0462449, 0.102734, -0.000159445, 0, 0, 0, 1),
            Qt.matrix4x4(0.0593366, 0.989962, -0.128276, -0.226188, -0.0957679, 0.133555, 0.986403, -0.0429914, 0.993634, -0.0462451, 0.102731, -0.000159361, 0, 0, 0, 1),
            Qt.matrix4x4(0.0717905, 0.96266, -0.261021, -0.299745, -0.0868268, 0.266734, 0.959851, -0.0844726, 0.993634, -0.0462447, 0.102734, -0.000159497, 0, 0, 0, 1),
            Qt.matrix4x4(0.052136, 0.997101, -0.0554134, -0.397215, -0.0998731, 0.0604167, 0.993164, -0.00179365, 0.993634, -0.0462453, 0.102733, -0.000159269, 0, 0, 0, 1),
            Qt.matrix4x4(0.985651, -0.15429, -0.0684685, 0.0373958, 0.100984, 0.213951, 0.971611, -0.081982, -0.135261, -0.964583, 0.226462, 0.384641, 0, 0, 0, 1),
            Qt.matrix4x4(0.788415, -0.589832, 0.174643, 0.0926977, -0.272261, -0.0800077, 0.958892, 0.0885573, -0.551612, -0.803553, -0.223668, 0.386452, 0, 0, 0, 1),
            Qt.matrix4x4(0.716768, -0.632828, 0.292869, 0.0227472, -0.436741, -0.0799974, 0.896023, 0.128777, -0.5436, -0.770149, -0.333722, 0.374363, 0, 0, 0, 1),
            Qt.matrix4x4(0.644705, -0.700983, 0.304926, -0.0503604, 0.619877, 0.712823, 0.328077, -0.385019, -0.447335, -0.0224966, 0.894084, 0.118375, 0, 0, 0, 1),
            Qt.matrix4x4(-0.96083, -0.0636989, -0.269718, 0.0377081, -0.277121, 0.231549, 0.932518, -0.0819213, 0.00305259, 0.970736, -0.240131, -0.38462, 0, 0, 0, 1),
            Qt.matrix4x4(-0.858261, -0.513195, 0.00439038, 0.0929622, 0.0622307, -0.0955749, 0.993475, 0.0885033, -0.509427, 0.852934, 0.113965, -0.386281, 0, 0, 0, 1),
            Qt.matrix4x4(-0.816522, -0.561467, 0.134339, 0.0229932, 0.235371, -0.111279, 0.965514, 0.128669, -0.527155, 0.819983, 0.223014, -0.374191, 0, 0, 0, 1),
            Qt.matrix4x4(-0.755012, -0.635839, 0.160206, -0.0501357, -0.605613, 0.769859, 0.201372, -0.384822, -0.251376, 0.0550157, -0.966325, -0.118263, 0, 0, 0, 1),
            Qt.matrix4x4(0.0590774, 0.990319, -0.125607, -0.396243, -0.0959311, 0.130877, 0.986747, -0.0294173, 0.993634, -0.0462448, 0.102734, -0.000159438, 0, 0, 0, 1),
            Qt.matrix4x4(0.052136, 0.997101, -0.055413, -0.5211, -0.0998731, 0.0604163, 0.993165, 0.00750447, 0.993634, -0.0462453, 0.102733, -0.000159192, 0, 0, 0, 1),
            Qt.matrix4x4(0.339831, 0.930243, 0.138433, -0.983872, -0.156166, -0.0893358, 0.983683, 0.166819, 0.92743, -0.355904, 0.114913, 0.144129, 0, 0, 0, 1),
            Qt.matrix4x4(-0.273977, 0.95881, 0.0749705, -0.983774, -0.0568354, -0.0939588, 0.993953, 0.166803, 0.960056, 0.268059, 0.0802368, -0.144432, 0, 0, 0, 1),
            Qt.matrix4x4(0.0757279, -0.400997, -0.912944, -0.0664403, 0.0834149, 0.914912, -0.394942, -0.165663, 0.993634, -0.046245, 0.102733, -0.000159459, 0, 0, 0, 1),
            Qt.matrix4x4(0.0521356, 0.997101, -0.0554131, 0.00566419, -0.0998559, 0.0604155, 0.993166, -0.0144963, 0.993635, -0.046246, 0.102716, -0.0657465, 0, 0, 0, 1),
            Qt.matrix4x4(0.0521356, 0.997101, -0.0554131, 0.00566423, -0.0998559, 0.0604155, 0.993166, -0.0144963, 0.993635, -0.046246, 0.102716, 0.0654275, 0, 0, 0, 1)
        ]
    }

    Node {
        id: bodyRigged
        rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)

        AccessoryMaterialLibrary {
            id: matLib
        }

        Node {
            id: universalArmature
            position: Qt.vector3d(-0.0012664840323850513, 0.011565704829990864, 0.004971697460860014)
            rotation: Qt.quaternion(0.997986, -0.0290157, -0.0246449, 0.0507453)

            Model {
                id: toyNodeModel
                source: root.currentElement.customMesh
                skin: skin
                materials: animationMaterial
            }
            Node {
                id: universalRigged
                position: Qt.vector3d(-1.0186340659856796e-10, 1.2951204553246498e-09, 3.41970007866621e-10)
                rotation: Qt.quaternion(0.707107, 5.39183e-08, 0.707107, -5.49885e-08)
            }
            Node {
                id: universalRiggedPelvis
                position: Qt.vector3d(-5.922629497945309e-09, -0.010523475706577301, -0.1347365826368332)
                rotation: Qt.quaternion(0.707106, -0.00123974, 0.707106, -0.00122761)
                Node {
                    id: universalRiggedLThigh
                    position: Qt.vector3d(-0.012414590455591679, 0.00680693332105875, 0.06434483826160431)
                    rotation: Qt.quaternion(-0.0164644, -0.0243565, 0.999568, -0.00033351)
                    Node {
                        id: universalRiggedLCalf
                        position: Qt.vector3d(0.05875241756439209, -4.958128556609154e-07, 3.4319236874580383e-07)
                        rotation: Qt.quaternion(0.989259, 0.0154112, -0.0247383, -0.143236)
                        Node {
                            id: universalRiggedLFoot
                            position: Qt.vector3d(0.05307980626821518, -1.7031561583280563e-07, 3.070454113185406e-08)
                            rotation: Qt.quaternion(0.674376, -0.00462269, 0.0147114, 0.738227)
                        }
                    }
                }
                Node {
                    id: universalRiggedRThigh
                    position: Qt.vector3d(-0.012414581142365932, 0.006806946359574795, -0.06434483826160431)
                    rotation: Qt.quaternion(0.0164537, -0.024393, 0.999567, 0.000334035)
                    Node {
                        id: universalRiggedRCalf
                        position: Qt.vector3d(0.05875237286090851, -4.461035132408142e-07, -3.611203283071518e-07)
                        rotation: Qt.quaternion(0.98929, -0.0154181, 0.024642, -0.143043)
                        Node {
                            id: universalRiggedRFoot
                            position: Qt.vector3d(0.05307982489466667, 1.555308699607849e-07, -1.2078089639544487e-08)
                            rotation: Qt.quaternion(0.674507, 0.00466852, -0.0146408, 0.738109)
                        }
                    }
                }
                Node {
                    id: universalRiggedSpine1
                    position: Qt.vector3d(0.006677897181361914, -0.0005109388148412108, -2.1245796233415604e-09)
                    rotation: Qt.quaternion(0.99049, -1.53498e-07, -2.71501e-08, 0.137586)
                    Node {
                        id: universalRiggedSpine2
                        position: Qt.vector3d(0.09655272960662842, 2.60770320892334e-08, -1.0433723218739033e-08)
                        rotation: Qt.quaternion(0.985061, 1.21159e-06, -1.42053e-07, -0.172207)
                        Node {
                            id: universalRiggedSpine3
                            position: Qt.vector3d(0.08225015550851822, -3.1548552215099335e-08, -1.0419171303510666e-08)
                            rotation: Qt.quaternion(0.997682, -1.23839e-06, -1.21353e-08, -0.068047)
                            Node {
                                id: universalRiggedRibcage
                                position: Qt.vector3d(0.08838807046413422, -4.6566128730773926e-08, 1.9266735762357712e-08)
                                rotation: Qt.quaternion(0.994504, 1.69735e-07, -3.02714e-07, 0.104698)
                                Node {
                                    id: universalRiggedLCollarbone
                                    position: Qt.vector3d(-0.00253615016117692, 0.0148304533213377, 0.004399856552481651)
                                    rotation: Qt.quaternion(0.671309, 0.00376807, -0.730239, -0.126811)
                                    Node {
                                        id: universalRiggedLUpperarm
                                        position: Qt.vector3d(0.14280031621456146, -7.450580596923828e-08, -7.613562047481537e-08)
                                        rotation: Qt.quaternion(0.941044, 0.206179, -0.240757, 0.118159)
                                        Node {
                                            id: universalRiggedLForearm
                                            position: Qt.vector3d(0.10050942748785019, -1.6065314412117004e-08, 7.89295881986618e-08)
                                            rotation: Qt.quaternion(0.995913, 0.0540124, -0.0201404, 0.0695285)
                                            Node {
                                                id: universalRiggedLPalm
                                                position: Qt.vector3d(0.11251094192266464, -1.4901161193847656e-08, -1.9837170839309692e-07)
                                                rotation: Qt.quaternion(0.693424, -0.718796, -0.0476581, -0.0149871)
                                                Node {
                                                    id: accessoryHolder_Hand_Left
                                                    position: Qt.vector3d(-0.008436787873506546, -0.2557981610298157, -0.12775936722755432)
                                                    rotation: Qt.quaternion(0.103318, 0.429085, 0.865849, -0.235621)
                                                    CustomModel {
                                                        id: metalBracelet_Left
                                                        position: Qt.vector3d(0.158, 0.048, -0.24153)
                                                        rotation: Qt.quaternion(0.707972, 0.0986974, 0.0879475, 0.693758)
                                                        visible: AccessoryState.metalBracelet_LeftVisible
                                                        source: "meshes/cylinder_009_mesh.mesh"
                                                        materials: matLib.items_generic
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                Node {
                                    id: universalRiggedRCollarbone
                                    position: Qt.vector3d(-0.002536127809435129, 0.014830454252660275, -0.004399869590997696)
                                    rotation: Qt.quaternion(0.671308, -0.00376816, 0.73024, -0.126811)
                                    Node {
                                        id: universalRiggedRUpperarm
                                        position: Qt.vector3d(0.14280030131340027, -1.862645149230957e-08, -6.52798917144537e-08)
                                        rotation: Qt.quaternion(0.941044, -0.206179, 0.240757, 0.118159)
                                        Node {
                                            id: universalRiggedRForearm
                                            position: Qt.vector3d(0.10050933808088303, 1.4901161193847656e-08, 1.7229467630386353e-08)
                                            rotation: Qt.quaternion(0.995913, -0.0540127, 0.0201401, 0.0695281)
                                            Node {
                                                id: universalRiggedRPalm
                                                position: Qt.vector3d(0.11251107603311539, 3.725290298461914e-08, -3.67872416973114e-08)
                                                rotation: Qt.quaternion(0.693424, 0.718796, 0.0476583, -0.0149872)
                                                Node {
                                                    id: accessoryHolder_Hand_Right
                                                    position: Qt.vector3d(-0.20556066930294037, -0.35123521089553833, 0.061212554574012756)
                                                    rotation: Qt.quaternion(0.568684, 0.349602, 0.211173, -0.713991)
                                                    scale: Qt.vector3d(0.999999, 1, 0.999999)
                                                    CustomModel {
                                                        id: metalBracelet_Right
                                                        position: Qt.vector3d(-0.19480888545513153, 0.04038756340742111, -0.3259944021701813)
                                                        rotation: Qt.quaternion(-0.0889828, 0.909527, -0.323377, 0.2455)
                                                        visible: AccessoryState.metalBracelet_RightVisible
                                                        source: "meshes/cylinder_009_mesh.mesh"
                                                        materials: matLib.items_generic
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                Node {
                                    id: universalRiggedNeck
                                    position: Qt.vector3d(0.0001160837709903717, -0.0005109547637403011, -9.851646609604359e-09)
                                    rotation: Qt.quaternion(0.999372, -4.15573e-07, 2.12591e-07, -0.0354308)
                                    Node {
                                        id: universalRiggedHead
                                        position: Qt.vector3d(0.12408043444156647, 4.0512531995773315e-08, -3.768946044147015e-09)
                                        rotation: Qt.quaternion(0.999372, 4.27221e-07, -2.21328e-07, 0.035431)
                                        Node {
                                            id: universalRiggedEar_L
                                            position: Qt.vector3d(0.4717787802219391, -1.1548399925231934e-07, 0.17517535388469696)
                                            rotation: Qt.quaternion(0.984193, -0.0126966, -0.157936, 0.0791197)
                                        }
                                        Node {
                                            id: universalRiggedEar_R
                                            position: Qt.vector3d(0.4717785120010376, -1.1734664440155029e-07, -0.17517589032649994)
                                            rotation: Qt.quaternion(0.984193, 0.0126967, 0.157936, 0.0791197)
                                        }

                                        EyesAccessories {
                                            id: eyesAccessories
                                            matLib: matLib
                                        }

                                        MouthAccessories {
                                            id: mouthAccessories
                                            currentElement: root.currentElement
                                            matLib: matLib
                                        }

                                        EyewearAccessories {
                                            id: eyewearAccessories
                                            matLib: matLib
                                        }

                                        HeadwearAccessories {
                                            id: headwearAccessories
                                            matLib: matLib
                                        }
                                    }
                                }
                            }
                            BodyAccessories {
                                id: bodyAccessories
                                matLib: matLib
                            }
                        }
                    }
                }
                Node {
                    id: universalRiggedTail
                    position: Qt.vector3d(0.0038796490989625454, -0.12033336609601974, 1.714215613901615e-08)
                    rotation: Qt.quaternion(0.573576, 5.51413e-08, -3.18342e-08, -0.819152)
                }
            }
            Node {
                id: universalRiggedLPlatform
                position: Qt.vector3d(0.06558699160814285, 0.0031835290137678385, 3.4924596548080444e-10)
                rotation: Qt.quaternion(0.707107, 1.77807e-08, 0.707107, -3.14455e-08)
            }
            Node {
                id: universalRiggedRPlatform
                position: Qt.vector3d(-0.06558700650930405, 0.003183548804372549, 7.8580342233181e-10)
                rotation: Qt.quaternion(0.707107, 1.77807e-08, 0.707107, -3.14455e-08)
            }
            Node {
                id: neutral_bone
            }
        }
    }

    AnimationMaterial {
        id: animationMaterial
        materialName: root.currentElement.name
        baseColorSource: root.currentElement.baseColor
        normalMapSource: root.currentElement.normalMap
    }
}
