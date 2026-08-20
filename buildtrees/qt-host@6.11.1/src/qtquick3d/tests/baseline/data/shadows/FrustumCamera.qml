// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: window
    visible: true
    width: 1200
    height: 720

    SceneEnvironment {
        id: sceneEnvironment
        clearColor: "lightblue"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }

    FrustumCamera {
        id: camera1
        eulerRotation: Qt.vector3d(-10.4172, 7.45044, 0)
        position: Qt.vector3d(381.119, 842.535, 955.253)

        clipNear: 100
        clipFar: 10000
        top: 10
        bottom: -10
        left: -10
        right: 10
    }

    Node {
        id: sceneA
        DirectionalLight {
            castsShadow: true
            eulerRotation: Qt.vector3d(-40, -120, 0)
            csmNumSplits: 2
            shadowMapQuality: Light.ShadowMapQualityLow
            csmBlendRatio: 0.05
            shadowBias: 20
            pcfFactor: 0
            softShadowQuality: Light.Hard
            shadowMapFar: camera1.clipFar
        }

        Model {
            id: ground
            source: "#Cube"
            scale: Qt.vector3d(25, 0.01, 135)
            z: -5500
            materials: DefaultMaterial {
                diffuseColor: "gray"
            }
            castsShadows: false
        }

        Node {
            Component.onCompleted: {

                var z_pos = 0
                for (var i = 0; i < 25; i++) {
                    var conesAndCylinderTrio = Qt.createQmlObject(`
                        import QtQuick
                        import QtQuick3D

                        Node {
                            property var z_positions: [` + z_pos + `,` + (z_pos -125) + `,` + (z_pos - 250) + `]

                            PrincipledMaterial {
                                id: material
                                baseColor: "gray"
                            }

                            Model {
                                source: "#Cone"
                                position: Qt.vector3d(0, 450, z_positions[0])
                                eulerRotation.z: 180
                                scale.y: 5
                                materials: material
                            }

                            Model {
                                source: "#Cone"
                                position.z: z_positions[1]
                                scale.y: 2.5
                                materials: material
                            }

                            Model {
                                source: "#Cylinder"
                                position: Qt.vector3d(0, 175, z_positions[2])
                                materials: material
                                scale.y: 3.5
                            }
                        }`,
                        this,
                        "snippet" + i
                    );
                    z_pos -= 450
                }
            }
        }
    }

    View3D {
        id: view
        width: parent.width
        height: parent.height
        camera: camera1
        environment: sceneEnvironment
        importScene: sceneA
    }
}
