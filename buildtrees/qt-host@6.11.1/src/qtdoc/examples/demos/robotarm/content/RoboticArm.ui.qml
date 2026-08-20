// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: rootNode
    property int rotation1
    property int rotation2
    property int rotation3
    property int rotation4
    property int clawsAngle

    readonly property alias hand_position: hand_grab_t.scenePosition
    readonly property alias hand_hinge_position: hand_hinge.scenePosition
    readonly property alias arm_position: arm.scenePosition
    readonly property alias forearm_position: forearm.scenePosition
    readonly property alias root_position: root.scenePosition

    Model {
        id: base
        scale.x: 100
        scale.y: 100
        scale.z: 100
        source: "meshes/base.mesh"
        eulerRotation.x: -90

        DefaultMaterial {
            id: steel_material
            diffuseColor: "#ff595959"
        }

        DefaultMaterial {
            id: plastic_material
        }
        materials: [steel_material, plastic_material]

        Model {
            id: root
            y: -5.96047e-08
            z: 1.0472
            eulerRotation.z: rootNode.rotation4
            source: "meshes/root.mesh"

            DefaultMaterial {
                id: plastic_color_material
                diffuseColor: "#41cd52"
            }
            materials: [plastic_material, plastic_color_material, steel_material]

            Model {
                id: forearm
                x: 5.32907e-15
                y: -0.165542
                z: 1.53472
                eulerRotation.x: rootNode.rotation3
                source: "meshes/forearm.mesh"
                materials: [plastic_material, steel_material]

                Model {
                    id: arm
                    x: -7.43453e-07
                    y: 0.667101
                    z: 2.23365
                    eulerRotation.x: rootNode.rotation2
                    source: "meshes/arm.mesh"

                    DefaultMaterial {
                        id: plastic_qt_material
                        diffuseMap: Texture {
                            source: "maps/qt.png"
                            pivotU: 0.5
                            pivotV: 0.5
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                    materials: [plastic_material, plastic_qt_material, steel_material]

                    Model {
                        id: hand_hinge
                        x: 7.43453e-07
                        y: 0.0635689
                        z: 2.12289
                        eulerRotation.x: rootNode.rotation1
                        source: "meshes/hand_hinge.mesh"
                        materials: [plastic_material]

                        Model {
                            id: hand
                            x: 3.35649e-06
                            y: 2.38419e-07
                            z: 0.366503
                            source: "meshes/hand.mesh"
                            materials: [plastic_material, steel_material]

                            Model {
                                id: hand_grab_t_hinge_2
                                x: -9.5112e-07
                                y: 0.323057
                                z: 0.472305
                                eulerRotation: hand_grab_t_hinge_1.eulerRotation
                                source: "meshes/hand_grab_t_hinge_2.mesh"
                                materials: [steel_material]
                            }

                            Model {
                                id: hand_grab_t_hinge_1
                                x: -9.3061e-07
                                y: 0.143685
                                z: 0.728553
                                eulerRotation.x: rootNode.clawsAngle * -1
                                source: "meshes/hand_grab_t_hinge_1.mesh"
                                materials: [steel_material]

                                Model {
                                    id: hand_grab_t
                                    x: -2.42588e-06
                                    y: -0.0327932
                                    z: 0.414757
                                    eulerRotation.x: hand_grab_t_hinge_1.eulerRotation.x * -1
                                    source: "meshes/hand_grab_t.mesh"
                                    materials: [plastic_color_material, steel_material]
                                }
                            }

                            Model {
                                id: hand_grab_b_hinge_1
                                x: -9.38738e-07
                                y: -0.143685
                                z: 0.728553
                                eulerRotation.x: rootNode.clawsAngle
                                source: "meshes/hand_grab_b_hinge_1.mesh"
                                materials: [steel_material]

                                Model {
                                    id: hand_grab_b
                                    x: -2.41775e-06
                                    y: 0.0327224
                                    z: 0.413965
                                    eulerRotation.x: hand_grab_b_hinge_1.eulerRotation.x * -1
                                    source: "meshes/hand_grab_b.mesh"
                                    materials: [plastic_color_material, steel_material]
                                }
                            }

                            Model {
                                id: hand_grab_b_hinge_2
                                x: -9.5112e-07
                                y: -0.323058
                                z: 0.472305
                                eulerRotation: hand_grab_b_hinge_1.eulerRotation
                                source: "meshes/hand_grab_b_hinge_2.mesh"
                                materials: [steel_material]
                            }
                        }
                    }
                }
            }
        }
    }
}
