// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

import QtQuick.Timeline

Node {
    id: node

    // Resources
    PrincipledMaterial {
        id: default_effect_material
        objectName: "Default-effect"
        baseColor: "#ffcccccc"
        roughness: 1
        alphaMode: PrincipledMaterial.Opaque
    }
    Skin {
        id: skin
        joints: [
            torso_joint_1,
            torso_joint_2,
            torso_joint_3,
            neck_joint_1,
            neck_joint_2,
            arm_joint_L_1,
            arm_joint_R_1,
            arm_joint_L_2,
            arm_joint_R_2,
            arm_joint_L_3,
            arm_joint_R_3,
            leg_joint_L_1,
            leg_joint_R_1,
            leg_joint_L_2,
            leg_joint_R_2,
            leg_joint_L_3,
            leg_joint_R_3,
            leg_joint_L_5,
            leg_joint_R_5
        ]
        inverseBindPoses: [
            Qt.matrix4x4(0.999983, 0.000442018, 0.00581419, -0.00398856, 0, 0.997123, -0.0758045, 0.0520021, -0.005831, 0.0758032, 0.997106, -0.684015, 0, 0, 0, 1),
            Qt.matrix4x4(1, 0, 0, 0, 0, -0.01376, 0.999905, -0.85674, 0, -0.999905, -0.0137601, 0.024791, 0, 0, 0, 1),
            Qt.matrix4x4(1, 0, 0, 0, 0, 0.979842, 0.199774, -0.224555, 0, -0.199774, 0.979842, -1.05133, 0, 0, 0, 1),
            Qt.matrix4x4(1, 0, 0, 0, 0, -0.00751853, 0.999972, -1.12647, 0, -0.999972, -0.00751847, 0.00796944, 0, 0, 0, 1),
            Qt.matrix4x4(-1, -1.50995e-07, 0, 0, 0, 0.00364935, 0.999993, -1.19299, -1.51869e-07, 0.999993, -0.00364941, 0.00535393, 0, 0, 0, 1),
            Qt.matrix4x4(-0.0623881, 0.998036, -0.00569177, 0.00162297, 0.891518, 0.0531644, -0.449853, 0.404156, -0.448667, -0.0331397, -0.893084, 0.998987, 0, 0, 0, 1),
            Qt.matrix4x4(0.109672, 0.988876, -0.100484, 0.107683, -0.891521, 0.0531632, -0.449849, 0.404152, -0.439503, 0.13892, 0.887434, -0.993169, 0, 0, 0, 1),
            Qt.matrix4x4(0.530194, 0.847874, 0.001751, -0.183428, 0.760039, -0.474352, -0.444218, 0.206564, -0.375811, 0.236853, -0.895917, 0.973213, 0, 0, 0, 1),
            Qt.matrix4x4(-0.0705104, -0.619322, 0.781965, -0.761146, -0.760038, -0.474352, -0.444223, 0.206569, 0.646043, -0.625645, -0.437261, 0.633599, 0, 0, 0, 1),
            Qt.matrix4x4(0.631434, 0.775418, -0.00419003, -0.228155, 0.649284, -0.53166, -0.543845, 0.154659, -0.423935, 0.340682, -0.839175, 0.951451, 0, 0, 0, 1),
            Qt.matrix4x4(0.111378, -0.773831, 0.623523, -0.550204, -0.649284, -0.531661, -0.543845, 0.15466, 0.752347, -0.344271, -0.561651, 0.809067, 0, 0, 0, 1),
            Qt.matrix4x4(-0.830471, -0.549474, 0.091635, -0.00030848, 0.0339727, -0.214148, -0.97621, 0.596867, 0.556025, -0.807601, 0.196511, -0.159297, 0, 0, 0, 1),
            Qt.matrix4x4(-0.994689, 0.102198, 0.0121981, -0.0750653, -0.0339737, -0.214147, -0.97621, 0.596867, -0.0971548, -0.97144, 0.216482, -0.140501, 0, 0, 0, 1),
            Qt.matrix4x4(-0.99973, 0.0232223, -7.82996e-05, 0.0784336, 0.0051282, 0.217484, -0.97605, 0.357951, -0.0226493, -0.975788, -0.217544, 0.0222206, 0, 0, 0, 1),
            Qt.matrix4x4(-0.998171, -0.0599068, -0.00810355, -0.0775425, -0.00512856, 0.217484, -0.97605, 0.357951, 0.0602345, -0.974224, -0.217393, 0.0251548, 0, 0, 0, 1),
            Qt.matrix4x4(-0.999327, 0.0366897, 0, 0.0783684, 0.0287104, 0.781987, 0.622632, -0.0567413, 0.0228442, 0.622213, -0.782514, 0.0634761, 0, 0, 0, 1),
            Qt.matrix4x4(-0.999326, 0.00828946, 0.0357652, -0.0814984, 0.0287402, 0.782804, 0.621604, -0.0521458, -0.0228444, 0.622213, -0.782514, 0.0634761, 0, 0, 0, 1),
            Qt.matrix4x4(0.994013, 0.109264, 0.000418345, -0.0755577, 0.109252, -0.993835, -0.0188101, -0.0405796, -0.00164008, 0.0187438, -0.999822, 0.0227357, 0, 0, 0, 1),
            Qt.matrix4x4(0.994011, -0.109281, 0.000483894, 0.0755372, -0.109253, -0.993836, -0.018811, -0.0405797, 0.00253636, 0.0186453, -0.999823, 0.0228038, 0, 0, 0, 1)
        ]
    }

    // Nodes:
    Node {
        id: z_UP
        objectName: "Z_UP"
        rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
        Node {
            id: armature
            objectName: "Armature"
            Node {
                id: torso_joint_1
                objectName: "torso_joint_1"
                position: Qt.vector3d(2.79397e-09, -1.41566e-07, 0.686)
                rotation: Qt.quaternion(0.999276, -0.0379294, -0.00291343, 0.000110585)
                scale: Qt.vector3d(1, 1, 1)
                Node {
                    id: torso_joint_2
                    objectName: "torso_joint_2"
                    position: Qt.vector3d(0.000999981, -4.84288e-08, 0.171491)
                    rotation: Qt.quaternion(0.674713, 0.738075, 0.00196715, -0.00215189)
                    scale: Qt.vector3d(1, 1, 1)
                    Node {
                        id: torso_joint_3
                        objectName: "torso_joint_3"
                        position: Qt.vector3d(0, 0.218018, 3.72529e-09)
                        rotation: Qt.quaternion(0.770153, -0.637859, 4.25872e-10, 3.52719e-10)
                        scale: Qt.vector3d(1, 1, 1)
                        Node {
                            id: neck_joint_1
                            objectName: "neck_joint_1"
                            position: Qt.vector3d(0, 7.45058e-08, 0.0525597)
                            rotation: Qt.quaternion(0.77214, 0.635452, 1.03679e-15, 8.51267e-16)
                            Node {
                                id: neck_joint_2
                                objectName: "neck_joint_2"
                                position: Qt.vector3d(0, 0.0665059, 0)
                                rotation: Qt.quaternion(-7.54967e-08, 4.21579e-10, 0.999984, -0.00558399)
                                scale: Qt.vector3d(1, 1, 1)
                            }
                        }
                        Node {
                            id: arm_joint_L_1
                            objectName: "arm_joint_L_1"
                            position: Qt.vector3d(0.0880006, -0.000199288, -0.000977397)
                            rotation: Qt.quaternion(-0.0885639, 0.678942, 0.687945, -0.240678)
                            scale: Qt.vector3d(1, 1, 1)
                            Node {
                                id: arm_joint_L_2
                                objectName: "arm_joint_L_2"
                                position: Qt.vector3d(1.86265e-09, 0.244526, -5.96046e-08)
                                rotation: Qt.quaternion(0.952132, -0.00240006, 0.139812, 0.271831)
                                Node {
                                    id: arm_joint_L_3
                                    objectName: "arm_joint_L_3"
                                    position: Qt.vector3d(0, 0.185517, 0)
                                    rotation: Qt.quaternion(0.996411, 0.0572906, 0.0282273, 0.0555599)
                                    scale: Qt.vector3d(1, 1, 1)
                                }
                            }
                        }
                        Node {
                            id: arm_joint_R_1
                            objectName: "arm_joint_R_1"
                            position: Qt.vector3d(-0.0880006, -0.000199288, -0.000977397)
                            rotation: Qt.quaternion(0.69168, -0.276431, -0.0518638, 0.665187)
                            scale: Qt.vector3d(1, 1, 1)
                            Node {
                                id: arm_joint_R_2
                                objectName: "arm_joint_R_2"
                                position: Qt.vector3d(-7.45058e-09, 0.244526, -5.96046e-08)
                                rotation: Qt.quaternion(-0.314075, -0.228006, 0.909648, -0.148023)
                                scale: Qt.vector3d(1, 0.999999, 1)
                                Node {
                                    id: arm_joint_R_3
                                    objectName: "arm_joint_R_3"
                                    position: Qt.vector3d(-5.96046e-08, 0.185517, 0)
                                    rotation: Qt.quaternion(0.986567, 0.0785489, -0.142535, 0.0141023)
                                    scale: Qt.vector3d(1, 1, 1)
                                }
                            }
                        }
                    }
                }
                Node {
                    id: leg_joint_L_1
                    objectName: "leg_joint_L_1"
                    position: Qt.vector3d(0.0676193, 0.00446109, -0.0722646)
                    rotation: Qt.quaternion(-0.201112, 0.210881, -0.624331, 0.724772)
                    scale: Qt.vector3d(1, 1, 1)
                    Node {
                        id: leg_joint_L_2
                        objectName: "leg_joint_L_2"
                        position: Qt.vector3d(0, 0.266112, 0)
                        rotation: Qt.quaternion(0.929599, -0.211154, 0.298432, 0.046886)
                        scale: Qt.vector3d(1, 1, 1)
                        Node {
                            id: leg_joint_L_3
                            objectName: "leg_joint_L_3"
                            position: Qt.vector3d(0, 0.275824, 0)
                            rotation: Qt.quaternion(0.530323, -0.847769, 0.00228158, 0.00633871)
                            scale: Qt.vector3d(1, 1, 1)
                            Node {
                                id: leg_joint_L_5
                                objectName: "leg_joint_L_5"
                                position: Qt.vector3d(-0.00234649, -0.0661733, 0.0278568)
                                rotation: Qt.quaternion(0.0687815, 0.0245321, -0.319997, 0.9446)
                                scale: Qt.vector3d(1, 1, 1)
                            }
                        }
                    }
                }
                Node {
                    id: leg_joint_R_1
                    objectName: "leg_joint_R_1"
                    position: Qt.vector3d(-0.0684572, 0.00446085, -0.0714711)
                    rotation: Qt.quaternion(0.0466302, -0.0233987, -0.654264, 0.754465)
                    scale: Qt.vector3d(1, 1, 1)
                    Node {
                        id: leg_joint_R_2
                        objectName: "leg_joint_R_2"
                        position: Qt.vector3d(0, 0.266112, 1.49012e-08)
                        rotation: Qt.quaternion(0.972955, -0.216061, -0.0810801, 0.01008)
                        Node {
                            id: leg_joint_R_3
                            objectName: "leg_joint_R_3"
                            position: Qt.vector3d(-7.45058e-09, 0.275824, -3.72529e-09)
                            rotation: Qt.quaternion(0.529777, -0.847167, 0.0320483, 0.0248404)
                            scale: Qt.vector3d(1, 1, 1)
                            Node {
                                id: leg_joint_R_5
                                objectName: "leg_joint_R_5"
                                position: Qt.vector3d(-0.00145853, -0.0661987, 0.0278568)
                                rotation: Qt.quaternion(-0.0414683, -0.0341433, -0.319178, 0.946171)
                                scale: Qt.vector3d(0.999999, 0.999999, 0.999999)
                            }
                        }
                    }
                }
            }
        }
        Model {
            id: proxy
            objectName: "Proxy"
            source: "riggedfigure.mesh"
            skin: skin
            materials: [
                default_effect_material
            ]
        }
    }

    // Animations:
    Timeline {
        id: timeline0
        objectName: "timeline0"
        property real framesPerSecond: 1000
        startFrame: 0
        endFrame: 1250
        currentFrame: 0
        enabled: true
        animations: TimelineAnimation {
            duration: 1250
            from: 0
            to: 1250
            running: true
            loops: Animation.Infinite
        }
        KeyframeGroup {
            target: leg_joint_R_5
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-0.00145852, -0.0661988, 0.0278567)
            }
        }
        KeyframeGroup {
            target: leg_joint_R_5
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.0414678, -0.0341418, -0.319178, 0.946171)
            }
        }
        KeyframeGroup {
            target: leg_joint_R_3
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(7.45058e-09, 0.275824, -7.45058e-09)
            }
        }
        KeyframeGroup {
            target: leg_joint_R_3
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.529778, 0.847166, -0.0320483, -0.0248404)
            }
        }
        KeyframeGroup {
            target: leg_joint_R_2
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(7.45058e-09, 0.266112, 0)
            }
        }
        KeyframeGroup {
            target: leg_joint_R_2
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.972956, 0.216062, 0.0810801, -0.01008)
            }
        }
        KeyframeGroup {
            target: leg_joint_R_1
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-0.0684572, 0.00446077, -0.0714709)
            }
        }
        KeyframeGroup {
            target: leg_joint_R_1
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(0.0466288, -0.0234008, -0.654263, 0.754465)
            }
        }
        KeyframeGroup {
            target: leg_joint_L_5
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-0.00234646, -0.0661734, 0.0278567)
            }
        }
        KeyframeGroup {
            target: leg_joint_L_5
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(0.0687827, 0.0245355, -0.319997, 0.9446)
            }
        }
        KeyframeGroup {
            target: leg_joint_L_3
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(0, 0.275824, -1.86265e-09)
            }
        }
        KeyframeGroup {
            target: leg_joint_L_3
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.530324, 0.847768, -0.00228158, -0.0063387)
            }
        }
        KeyframeGroup {
            target: leg_joint_L_2
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(0, 0.266112, 1.49012e-08)
            }
        }
        KeyframeGroup {
            target: leg_joint_L_2
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.929599, 0.211155, -0.298433, -0.046886)
            }
        }
        KeyframeGroup {
            target: leg_joint_L_1
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(0.067619, 0.00446084, -0.0722642)
            }
        }
        KeyframeGroup {
            target: arm_joint_R_3
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(0, 0.185517, 0)
            }
        }
        KeyframeGroup {
            target: arm_joint_R_3
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.986567, -0.078549, 0.142535, -0.0141023)
            }
        }
        KeyframeGroup {
            target: arm_joint_R_2
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-7.45058e-09, 0.244526, 0)
            }
        }
        KeyframeGroup {
            target: arm_joint_L_3
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(0, 0.185517, 5.96046e-08)
            }
        }
        KeyframeGroup {
            target: arm_joint_L_3
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.996411, -0.0572907, -0.0282272, -0.0555601)
            }
        }
        KeyframeGroup {
            target: arm_joint_L_2
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-2.32831e-09, 0.244526, 0)
            }
        }
        KeyframeGroup {
            target: arm_joint_L_2
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.952132, 0.00239999, -0.139812, -0.271831)
            }
        }
        KeyframeGroup {
            target: arm_joint_L_1
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(0.0880001, -0.00019978, -0.0009799)
            }
            Keyframe {
                frame: 1250
                value: Qt.vector3d(0.0879999, -0.000199795, -0.00098002)
            }
        }
        KeyframeGroup {
            target: arm_joint_L_1
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.321235, 0.618298, 0.538041, -0.474371)
            }
            Keyframe {
                frame: 1250
                value: Qt.quaternion(-0.0885618, 0.678943, 0.687945, -0.240675)
            }
        }
        KeyframeGroup {
            target: neck_joint_2
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-5.32907e-15, 0.0665059, 9.31323e-10)
            }
        }
        KeyframeGroup {
            target: neck_joint_2
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-1.1912e-07, -5.47696e-10, 0.999487, 0.0320266)
            }
            Keyframe {
                frame: 1250
                value: Qt.quaternion(-1.19207e-07, 6.65661e-10, 0.999984, -0.00558399)
            }
        }
        KeyframeGroup {
            target: neck_joint_1
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-8.88178e-15, -1.49012e-08, 0.0525595)
            }
            Keyframe {
                frame: 1250
                value: Qt.vector3d(-8.88178e-15, -4.47035e-08, 0.0525594)
            }
        }
        KeyframeGroup {
            target: neck_joint_1
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.686502, -0.727128, 6.39277e-11, -1.96486e-11)
            }
            Keyframe {
                frame: 1250
                value: Qt.quaternion(-0.77214, -0.635453, 1.20456e-13, 1.01944e-13)
            }
        }
        KeyframeGroup {
            target: torso_joint_2
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(0.000999983, -1.86265e-08, 0.171491)
            }
        }
        KeyframeGroup {
            target: torso_joint_2
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.674713, -0.738074, -0.00196715, 0.00215188)
            }
        }
        KeyframeGroup {
            target: torso_joint_3
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-7.10543e-15, 0.218018, -1.86265e-09)
            }
        }
        KeyframeGroup {
            target: torso_joint_3
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.770153, 0.637859, -4.26135e-10, -3.52842e-10)
            }
        }
        KeyframeGroup {
            target: arm_joint_R_1
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(-0.0880001, -0.000199795, -0.00098002)
            }
        }
        KeyframeGroup {
            target: arm_joint_R_1
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.611424, 0.495758, 0.309245, -0.533621)
            }
            Keyframe {
                frame: 1250
                value: Qt.quaternion(-0.691681, 0.276426, 0.0518633, -0.665189)
            }
        }
        KeyframeGroup {
            target: torso_joint_1
            property: "position"
            Keyframe {
                frame: 0
                value: Qt.vector3d(4.58966e-10, -1.15066e-07, 0.686)
            }
        }
        KeyframeGroup {
            target: torso_joint_1
            property: "rotation"
            Keyframe {
                frame: 0
                value: Qt.quaternion(-0.999276, 0.0379299, 0.00291355, -0.000113405)
            }
        }
    }
}
