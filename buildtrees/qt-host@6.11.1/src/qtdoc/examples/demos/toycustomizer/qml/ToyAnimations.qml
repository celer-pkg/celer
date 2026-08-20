// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick.Timeline

Node {
    id: node

    property alias isAnimationRunning: action_timeline.enabled

    scale.x: 110
    scale.y: 110
    scale.z: 110

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
            universalRiggedRPlatform,
            neutral_bone
        ]
        inverseBindPoses: [
            Qt.matrix4x4(1.07284e-07, 1, -7.54979e-08, 0, 1.50996e-07, 7.54979e-08, 1, 0, 1, -1.07284e-07, -1.50996e-07, 0, 0, 0, 0, 1),
            Qt.matrix4x4(2.81649e-07, 0.999994, -0.00348939, -0.134773, -1.70817e-05, 0.00348939, 0.999994, 0.0100533, 1, -2.21842e-07, 1.71398e-05, 2.17255e-07, 0, 0, 0, 1),
            Qt.matrix4x4(0.0329314, -0.998435, -0.0451968, 0.119869, 0.000118235, -0.0452174, 0.998977, 0.00919296, -0.999458, -0.032903, -0.00137108, 0.0683303, 0, 0, 0, 1),
            Qt.matrix4x4(-0.0129998, -0.94486, -0.327215, 0.0589647, -0.0281369, -0.326768, 0.944686, 0.0286533, -0.99952, 0.0214876, -0.0223376, 0.0647375, 0, 0, 0, 1),
            Qt.matrix4x4(-0.000182505, -0.240479, 0.970654, 0.0262676, -4.02055e-07, 0.970654, 0.240479, -0.00743665, -1, 4.34941e-05, -0.000177303, 0.0655843, 0, 0, 0, 1),
            Qt.matrix4x4(-0.0329086, -0.998433, -0.0452707, 0.11987, -0.000152004, -0.0452902, 0.998974, 0.00920188, -0.999459, 0.0328817, 0.00133861, -0.0683281, 0, 0, 0, 1),
            Qt.matrix4x4(0.0128422, -0.944968, -0.326912, 0.0589611, 0.0281007, -0.326469, 0.94479, 0.0286387, -0.999523, -0.0213196, 0.0223616, -0.064747, 0, 0, 0, 1),
            Qt.matrix4x4(0.00019049, -0.240512, 0.970646, 0.0262574, -4.13122e-07, 0.970646, 0.240512, -0.00743281, -1, -4.621e-05, 0.000184742, -0.0655914, 0, 0, 0, 1),
            Qt.matrix4x4(-4.3729e-06, 0.963085, 0.269196, -0.133216, -1.68022e-05, -0.269196, 0.963086, 0.0487172, 1, -3.11399e-07, 1.74163e-05, 2.3496e-07, 0, 0, 0, 1),
            Qt.matrix4x4(1.49192e-06, 0.997294, -0.0735157, -0.232669, -1.48438e-05, 0.0735157, 0.997294, -0.0321257, 1, -3.96436e-07, 1.49705e-05, 2.99266e-07, 0, 0, 0, 1),
            Qt.matrix4x4(3.70553e-06, 0.978076, -0.208246, -0.307641, -1.69669e-05, 0.208246, 0.978077, -0.0745874, 1, -9.08096e-08, 1.74238e-05, 1.90677e-07, 0, 0, 0, 1),
            Qt.matrix4x4(6.98522e-07, 1, -5.82125e-07, -0.402879, -1.70819e-05, 6.11941e-07, 1, 0.00951902, 1, -6.98312e-07, 1.71392e-05, 4.26681e-07, 0, 0, 0, 1),
            Qt.matrix4x4(0.979479, -0.0986618, -0.175745, 0.0361226, 0.190248, 0.164756, 0.967813, -0.0719363, -0.066531, -0.981387, 0.180145, 0.392227, 0, 0, 0, 1),
            Qt.matrix4x4(0.828613, -0.556696, 0.0590866, 0.0966515, -0.168334, -0.147105, 0.974692, 0.10039, -0.533915, -0.817588, -0.215604, 0.388558, 0, 0, 0, 1),
            Qt.matrix4x4(0.771554, -0.609853, 0.181062, 0.0283729, -0.338226, -0.152186, 0.928678, 0.14009, -0.538803, -0.777765, -0.323688, 0.37502, 0, 0, 0, 1),
            Qt.matrix4x4(0.704339, -0.682236, 0.196114, -0.0441651, 0.616666, 0.724895, 0.307003, -0.385554, -0.35161, -0.0952973, 0.931283, 0.129394, 0, 0, 0, 1),
            Qt.matrix4x4(-0.979473, -0.0986614, -0.175779, 0.0361222, -0.190281, 0.164756, 0.967806, -0.0719362, -0.0665246, 0.981387, -0.180147, -0.392227, 0, 0, 0, 1),
            Qt.matrix4x4(-0.828615, -0.556696, 0.0590578, 0.0966513, 0.168301, -0.147105, 0.974698, 0.10039, -0.533922, 0.817588, 0.215586, -0.388559, 0, 0, 0, 1),
            Qt.matrix4x4(-0.771561, -0.609853, 0.181035, 0.0283724, 0.338194, -0.152187, 0.92869, 0.140091, -0.538813, 0.777765, 0.32367, -0.375021, 0, 0, 0, 1),
            Qt.matrix4x4(-0.704346, -0.682236, 0.196089, -0.0441655, -0.616676, 0.724895, 0.306981, -0.385554, -0.351578, 0.0952975, -0.931296, -0.129394, 0, 0, 0, 1),
            Qt.matrix4x4(1.49828e-06, 0.997489, -0.0708174, -0.402694, -1.78674e-05, 0.0708175, 0.997489, -0.0185341, 1, -2.29001e-07, 1.79858e-05, 2.47119e-07, 0, 0, 0, 1),
            Qt.matrix4x4(6.98443e-07, 1, 4.37236e-08, -0.526764, -1.7082e-05, -6.45496e-09, 1, 0.0188172, 1, -6.98243e-07, 1.71392e-05, 4.91228e-07, 0, 0, 0, 1),
            Qt.matrix4x4(0.308868, 0.937592, 0.159754, -0.987326, -0.0500005, -0.151727, 0.987157, 0.178838, 0.94979, -0.312889, 1.63652e-05, 0.146053, 0, 0, 0, 1),
            Qt.matrix4x4(-0.308873, 0.937592, 0.159743, -0.987326, 0.0499668, -0.151728, 0.987159, 0.178838, 0.94979, 0.312888, 1.60746e-05, -0.146052, 0, 0, 0, 1),
            Qt.matrix4x4(1.6123e-05, -0.345298, -0.938493, -0.0751014, 6.23597e-06, 0.938493, -0.345297, -0.174885, 1, -2.85001e-07, 1.73418e-05, 2.4189e-07, 0, 0, 0, 1),
            Qt.matrix4x4(3.5862e-07, 1, -1.19209e-07, -2.30753e-08, 8.84148e-08, 1.19209e-07, 1, -0.00318353, 1, -3.5862e-07, -4.47035e-08, -0.065587, 0, 0, 0, 1),
            Qt.matrix4x4(3.5862e-07, 1, -1.19209e-07, 2.39664e-08, 8.84148e-08, 1.19209e-07, 1, -0.00318354, 1, -3.5862e-07, -4.47035e-08, 0.065587, 0, 0, 0, 1),
            Qt.matrix4x4(1, 0, 0, 0, 0, 1, -7.54979e-08, 0, 0, 7.54979e-08, 1, 0, 0, 0, 0, 1)
        ]
    }

    property string currentAnimal: "Bear"
    property int currentIndex: 0
    property var currentElement: AnimationModel.get(node.currentIndex)

    // Nodes:
    Node {
        id: characterWrapper
        eulerRotation.z: -90
        eulerRotation.y: 0
        eulerRotation.x: 90
        objectName: "CharacterWrapper"
        scale.x: 1
        scale.y: 1
        scale.z: 1
        Node {
            id: universalArmature
            x: 0
            y: 0
            eulerRotation.z: 0
            z: 0
            objectName: "UniversalArmature"
            Model {
                id: toyModel
                objectName: node.currentElement.name
                source: node.currentElement.meshSource
                skin: skin
                materials: [animationMaterial]
            }
            Node {
                id: universalRigged
                objectName: "UniversalRigged"
                rotation: Qt.quaternion(0.707107, 5.33851e-08, 0.707107, -5.33851e-08)
                scale.x: 1
                scale.y: 1
                scale.z: 1
            }
            Node {
                id: universalRiggedPelvis
                objectName: "UniversalRiggedPelvis"
                x: -6.994792745018685e-09
                y: -0.010523476637899876
                z: -0.13473661243915558
                rotation: Qt.quaternion(0.707106, -0.0012397, 0.707106, -0.0012276)
                Node {
                    id: universalRiggedLThigh
                    objectName: "UniversalRiggedLThigh"
                    x: -0.012414610013365746
                    y: 0.006806946825236082
                    z: 0.06434482336044312
                    rotation: Qt.quaternion(-0.0164644, -0.0243565, 0.999568, -0.000333505)
                    scale.x: 1
                    scale.y: 1
                    scale.z: 1
                    Node {
                        id: universalRiggedLCalf
                        objectName: "UniversalRiggedLCalf"
                        x: 0.0587523989379406
                        y: -5.097763278172351e-07
                        z: 3.21659172186628e-07
                        rotation: Qt.quaternion(0.989259, 0.0154112, -0.0247382, -0.143236)
                        scale.x: 1
                        scale.y: 1
                        scale.z: 1
                        Node {
                            id: universalRiggedLFoot
                            objectName: "UniversalRiggedLFoot"
                            x: 0.05307983607053757
                            y: -1.4668330550193787e-07
                            z: 5.8906152844429016e-08
                            rotation: Qt.quaternion(0.674376, -0.00462264, 0.0147112, 0.738227)
                            scale.x: 1
                            scale.y: 1
                            scale.z: 1
                        }
                    }
                }
                Node {
                    id: universalRiggedRThigh
                    objectName: "UniversalRiggedRThigh"
                    x: -0.012414601631462574
                    y: 0.006806947290897369
                    z: -0.06434483081102371
                    rotation: Qt.quaternion(0.0164535, -0.024393, 0.999567, 0.00033403)
                    scale.x: 1
                    scale.y: 1
                    scale.z: 1
                    Node {
                        id: universalRiggedRCalf
                        objectName: "UniversalRiggedRCalf"
                        x: 0.0587523989379406
                        y: -4.7176217776723206e-07
                        z: -3.521563485264778e-07
                        rotation: Qt.quaternion(0.98929, -0.0154181, 0.0246419, -0.143042)
                        scale.x: 1
                        scale.y: 1
                        scale.z: 1
                        Node {
                            id: universalRiggedRFoot
                            objectName: "UniversalRiggedRFoot"
                            x: 0.05307980626821518
                            y: 1.484295353293419e-07
                            z: -4.5693013817071915e-09
                            rotation: Qt.quaternion(0.674507, 0.00466854, -0.0146409, 0.738109)
                        }
                    }
                }
                Node {
                    id: universalRiggedSpine1
                    objectName: "UniversalRiggedSpine1"
                    x: 0.0066779013723134995
                    y: -0.0005109308403916657
                    z: -1.9820083352328766e-09
                    rotation: Qt.quaternion(0.99049, -1.4303e-07, -2.58273e-08, 0.137586)
                    scale.x: 1
                    scale.y: 1
                    scale.z: 1
                    Node {
                        id: universalRiggedSpine2
                        objectName: "UniversalRiggedSpine2"
                        x: 0.09655272215604782
                        y: 6.240583871885974e-08
                        z: -7.847475558264705e-09
                        rotation: Qt.quaternion(0.985061, 1.21264e-06, -1.63773e-07, -0.172207)
                        scale.x: 1
                        scale.y: 1
                        scale.z: 1
                        Node {
                            id: universalRiggedSpine3
                            objectName: "UniversalRiggedSpine3"
                            x: 0.08225009590387344
                            y: -2.5141007498064027e-08
                            z: -9.923056154548249e-09
                            rotation: Qt.quaternion(0.997682, -1.23592e-06, -2.19286e-08, -0.0680469)
                            scale.x: 1
                            scale.y: 1
                            scale.z: 1
                            Node {
                                id: universalRiggedRibcage
                                objectName: "UniversalRiggedRibcage"
                                x: 0.08838807046413422
                                y: -4.5635108136821145e-08
                                z: 6.035977051510599e-09
                                rotation: Qt.quaternion(0.994504, 1.73331e-07, -2.87182e-07, 0.104698)
                                scale.x: 1
                                scale.y: 1
                                scale.z: 1
                                Node {
                                    id: universalRiggedLCollarbone
                                    objectName: "UniversalRiggedLCollarbone"
                                    x: -0.002536088228225708
                                    y: 0.014830459840595722
                                    z: 0.004399864934384823
                                    rotation: Qt.quaternion(0.671309, 0.00376812, -0.730239, -0.126811)
                                    Node {
                                        id: universalRiggedLUpperarm
                                        objectName: "UniversalRiggedLUpperarm"
                                        x: 0.1428002566099167
                                        y: -8.009374141693115e-08
                                        z: 2.2118911147117615e-08
                                        rotation: Qt.quaternion(0.941044, 0.206179, -0.240757, 0.118159)
                                        scale.x: 1
                                        scale.y: 1
                                        scale.z: 1
                                        Node {
                                            id: universalRiggedLForearm
                                            objectName: "UniversalRiggedLForearm"
                                            x: 0.10050937533378601
                                            y: -1.0244548320770264e-08
                                            z: 2.6775524020195007e-08
                                            rotation: Qt.quaternion(0.995913, 0.0540123, -0.0201404, 0.0695284)
                                            scale.x: 1
                                            scale.y: 1
                                            scale.z: 1
                                            Node {
                                                id: universalRiggedLPalm
                                                objectName: "UniversalRiggedLPalm"
                                                x: 0.11251097172498703
                                                y: -8.381903171539307e-09
                                                z: -2.2444874048233032e-07
                                                rotation: Qt.quaternion(0.693424, -0.718796, -0.0476581, -0.0149872)
                                                scale.x: 1
                                                scale.y: 1
                                                scale.z: 1
                                            }
                                        }
                                    }
                                }
                                Node {
                                    id: universalRiggedRCollarbone
                                    objectName: "UniversalRiggedRCollarbone"
                                    x: -0.002536094281822443
                                    y: 0.014830457977950573
                                    z: -0.004399862140417099
                                    rotation: Qt.quaternion(0.671308, -0.00376826, 0.73024, -0.126811)
                                    scale.x: 1
                                    scale.y: 1
                                    scale.z: 1
                                    Node {
                                        id: universalRiggedRUpperarm
                                        objectName: "UniversalRiggedRUpperarm"
                                        x: 0.1428002417087555
                                        y: -3.91155481338501e-08
                                        z: -8.917413651943207e-08
                                        rotation: Qt.quaternion(0.941044, -0.206179, 0.240757, 0.118159)
                                        scale.x: 1
                                        scale.y: 1
                                        scale.z: 1
                                        Node {
                                            id: universalRiggedRForearm
                                            objectName: "UniversalRiggedRForearm"
                                            x: 0.1005094051361084
                                            y: 3.259629011154175e-08
                                            z: -3.655441105365753e-08
                                            rotation: Qt.quaternion(0.995913, -0.0540126, 0.0201401, 0.0695282)
                                            scale.x: 1
                                            scale.y: 1
                                            scale.z: 1
                                            Node {
                                                id: universalRiggedRPalm
                                                objectName: "UniversalRiggedRPalm"
                                                x: 0.1125110536813736
                                                y: 8.614733815193176e-08
                                                z: -1.3969838619232178e-09
                                                rotation: Qt.quaternion(0.693423, 0.718796, 0.0476582, -0.0149872)
                                                scale.x: 1
                                                scale.y: 1
                                                scale.z: 1
                                            }
                                        }
                                    }
                                }
                                Node {
                                    id: universalRiggedNeck
                                    objectName: "UniversalRiggedNeck"
                                    x: 0.00011611568334046751
                                    y: -0.0005109405028633773
                                    z: -1.0750227374956012e-09
                                    rotation: Qt.quaternion(0.999372, -4.31384e-07, 2.19509e-07, -0.0354307)
                                    Node {
                                        id: universalRiggedHead
                                        objectName: "UniversalRiggedHead"
                                        x: 0.12408063560724258
                                        y: 4.6562572464381446e-08
                                        z: -1.2859999287684332e-08
                                        rotation: Qt.quaternion(0.999372, 4.3134e-07, -2.19476e-07, 0.035431)
                                        scale.x: 1
                                        scale.y: 1
                                        scale.z: 1
                                        Node {
                                            id: universalRiggedEar_L
                                            objectName: "UniversalRiggedEar_L"
                                            x: 0.4717785120010376
                                            y: -5.7684662380097507e-08
                                            z: 0.17517533898353577
                                            rotation: Qt.quaternion(0.984193, -0.0126966, -0.157936, 0.0791197)
                                            scale.x: 1
                                            scale.y: 1
                                            scale.z: 1
                                        }
                                        Node {
                                            id: universalRiggedEar_R
                                            objectName: "UniversalRiggedEar_R"
                                            x: 0.4717782139778137
                                            y: -5.952252379870515e-08
                                            z: -0.17517590522766113
                                            rotation: Qt.quaternion(0.984193, 0.0126967, 0.157937, 0.0791197)
                                            scale.x: 1
                                            scale.y: 1
                                            scale.z: 1
                                        }
                                        Model {
                                            id: eyesModel
                                            objectName: node.currentElement.name + "Eyes"
                                            position: Qt.vector3d(node.currentElement.x, node.currentElement.y, node.currentElement.z)
                                            rotation: Qt.quaternion(node.currentElement.rotX, node.currentElement.rotY, node.currentElement.rotZ, node.currentElement.rotW)
                                            scale: Qt.vector3d(1, 1, 1)
                                            source: node.currentElement.eyesMeshSource
                                            materials: [ defaultEyes_material ]
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                Node {
                    id: universalRiggedTail
                    objectName: "UniversalRiggedTail"
                    x: 0.003879645373672247
                    y: -0.12033335864543915
                    z: 1.053022558039629e-08
                    rotation: Qt.quaternion(0.573576, -3.17139e-08, -1.00964e-07, -0.819152)
                    scale.x: 1
                    scale.y: 1
                    scale.z: 1
                }
            }
            Node {
                id: universalRiggedLPlatform
                objectName: "UniversalRiggedLPlatform"
                x: 0.06558700650930405
                y: 0.003183528082445264
                z: 3.0639199510851256e-10
                rotation: Qt.quaternion(0.707107, 1.58051e-08, 0.707107, -3.12594e-08)
                scale.x: 1
                scale.y: 1
                scale.z: 1
            }
            Node {
                id: universalRiggedRPlatform
                objectName: "UniversalRiggedRPlatform"
                x: -0.06558700650930405
                y: 0.003183547640219331
                z: 3.0639549231104013e-10
                rotation: Qt.quaternion(0.707107, 1.58051e-08, 0.707107, -3.12594e-08)
                scale.x: 1
                scale.y: 1
                scale.z: 1
            }
            Node {
                id: neutral_bone
                objectName: "neutral_bone"
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale.x: 1
                scale.y: 1
                scale.z: 1
            }
        }
    }

    Timeline {
        id: action_timeline
        objectName: "Action"
        property real framesPerSecond: 1000
        startFrame: 33
        endFrame: 8400
        enabled: node.isAnimationRunning
        onEnabledChanged: action_timeline.enabled ? bearAnim.resume() : bearAnim.pause()
        animations: [
            TimelineAnimation {
                id: bearAnim
                loops: 1
                onFinished: {
                    node.currentIndex = (node.currentIndex + 1) % AnimationModel.count
                    node.currentAnimal = node.currentElement.name
                }
                duration: 8400
                from: 33
                to: 8400
                running: node.currentAnimal  === node.currentElement.name
            }
        ]
        KeyframeGroup {
            target: universalRiggedRPlatform
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(-0.0655871, 0.00318354, 3.06424e-10)
            }
        }
        KeyframeGroup {
            target: universalRiggedRPlatform
            property: "rotation"
            Keyframe {
                frame: 33.3333
                value: Qt.quaternion(0.707107, -3.50759e-10, 0.707107, -4.74152e-08)
            }
        }
        KeyframeGroup {
            target: universalRiggedLPlatform
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(0.065587, 0.00318353, 3.06213e-10)
            }
        }
        KeyframeGroup {
            target: universalRiggedLPlatform
            property: "rotation"
            Keyframe {
                frame: 33.3333
                value: Qt.quaternion(0.707107, -3.50751e-10, 0.707107, -4.74152e-08)
            }
        }
        KeyframeGroup {
            target: universalRiggedTail
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(0.0038816, -0.120333, 1.77774e-08)
            }
        }
        KeyframeGroup {
            target: universalRiggedTail
            property: "rotation"
            Keyframe {
                frame: 33.3333
                value: Qt.quaternion(0.573576, -9.22992e-08, -4.89722e-08, -0.819152)
            }
        }
        KeyframeGroup {
            target: universalRiggedHead
            property: "rotation"
            keyframeSource: "animations/universalRiggedHead_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedNeck
            property: "rotation"
            keyframeSource: "animations/universalRiggedNeck_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedRForearm
            property: "rotation"
            keyframeSource: "animations/universalRiggedRForearm_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedRUpperarm
            property: "rotation"
            keyframeSource: "animations/universalRiggedRUpperarm_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedRCollarbone
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(-0.00253218, 0.01483, -0.00440005)
            }
        }
        KeyframeGroup {
            target: universalRiggedRCollarbone
            property: "rotation"
            keyframeSource: "animations/universalRiggedRCollarbone_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedLForearm
            property: "rotation"
            keyframeSource: "animations/universalRiggedLForearm_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedLUpperarm
            property: "rotation"
            keyframeSource: "animations/universalRiggedLUpperarm_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedLCollarbone
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(-0.00253258, 0.0148297, 0.00439969)
            }
        }
        KeyframeGroup {
            target: universalRiggedLCollarbone
            property: "rotation"
            keyframeSource: "animations/universalRiggedLCollarbone_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedRibcage
            property: "rotation"
            keyframeSource: "animations/universalRiggedRibcage_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedSpine3
            property: "rotation"
            keyframeSource: "animations/universalRiggedSpine3_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedSpine2
            property: "rotation"
            keyframeSource: "animations/universalRiggedSpine2_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedRFoot
            property: "rotation"
            Keyframe {
                frame: 33.3333
                value: Qt.quaternion(0.674392, 0.00483803, -0.0149455, 0.738206)
            }
        }
        KeyframeGroup {
            target: universalRiggedRCalf
            property: "rotation"
            keyframeSource: "animations/universalRiggedRCalf_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedRThigh
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(-0.0124157, 0.00680695, -0.0643431)
            }
        }
        KeyframeGroup {
            target: universalRiggedRThigh
            property: "rotation"
            keyframeSource: "animations/universalRiggedRThigh_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedLFoot
            property: "rotation"
            Keyframe {
                frame: 33.3333
                value: Qt.quaternion(0.674396, -0.00487579, 0.014913, 0.738203)
            }
        }
        KeyframeGroup {
            target: universalRiggedPelvis
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(-6.99518e-09, -0.0105235, -0.134737)
            }
        }
        KeyframeGroup {
            target: universalRiggedPelvis
            property: "rotation"
            Keyframe {
                frame: 33.3333
                value: Qt.quaternion(0.707106, -0.0012397, 0.707106, -0.00122736)
            }
        }
        KeyframeGroup {
            target: universalRiggedLCalf
            property: "rotation"
            keyframeSource: "animations/universalRiggedLCalf_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedLThigh
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(-0.0124117, 0.00680699, 0.0643468)
            }
        }
        KeyframeGroup {
            target: universalRiggedLThigh
            property: "rotation"
            keyframeSource: "animations/universalRiggedLThigh_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRiggedSpine1
            property: "position"
            Keyframe {
                frame: 33.3333
                value: Qt.vector3d(0.00668098, -0.000511268, -1.94761e-09)
            }
        }
        KeyframeGroup {
            target: universalRiggedSpine1
            property: "rotation"
            keyframeSource: "animations/universalRiggedSpine1_rotation_0.qad"
        }
        KeyframeGroup {
            target: universalRigged
            property: "rotation"
            Keyframe {
                frame: 33.3333
                value: Qt.quaternion(0.707107, 1.31358e-07, 0.707107, -1.52432e-07)
            }
        }

        KeyframeGroup {
            target: universalArmature
            property: "y"
            Keyframe {
                value: 1.5
                frame: 8400
            }

            Keyframe {
                value: -1.7
                frame: 33
            }

            Keyframe {
                value: 0
                frame: 2557
            }

            Keyframe {
                value: 0
                frame: 5838
            }
        }

        KeyframeGroup {
            target: universalArmature
            property: "eulerRotation.z"
            Keyframe {
                value: 0
                frame: 2557
            }

            Keyframe {
                value: 90
                frame: 3088
            }

            Keyframe {
                value: 90
                frame: 5464
            }

            Keyframe {
                value: 0
                frame: 5838
            }
        }
    }

    Node {
        id: __materialLibrary__

        AnimationMaterial {
            id: animationMaterial
            materialName: node.currentElement.name
            baseColorSource: node.currentElement.baseColor
            normalMapSource: node.currentElement.normalMap
        }

        PrincipledMaterial {
            id: defaultEyes_material
            objectName: "DefaultEyes"
            baseColor: "#ff040404"
            roughness: 0.3590908944606781
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
        }
    }
}
