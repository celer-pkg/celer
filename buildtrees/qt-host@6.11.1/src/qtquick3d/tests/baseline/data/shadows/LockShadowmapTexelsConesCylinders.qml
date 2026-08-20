// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: window
    visible: true
    width: 1200
    height: 720

    PerspectiveCamera {
        id: camera1
        position: Qt.vector3d(176.1126, 982.409, 0)
        eulerRotation: Qt.vector3d(-55.3464, -1.051, 0)
        clipFar: 15000
    }

    SceneEnvironment {
        id: environment1
        clearColor: "lightblue"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }

    property list<int> offsets: [0, width/2]

    function addConesCylindersTriple(parentNode, z_pos) {
        const newObject = Qt.createQmlObject(`
            import QtQuick
            import QtQuick3D

            Node {
                property var z_positions: [0, 100, 200]

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
            }
            `,
            parentNode,
            "ConesCylinders"
        );
        newObject.z_positions = [z_pos, z_pos - 125, z_pos - 250];
    }

    View3D {
        width: parent.width/2
        height: parent.height
        x: offsets[0]
        camera: camera1
        environment: environment1
        DirectionalLight {
            castsShadow: true
            shadowFactor: 100
            eulerRotation: Qt.vector3d(-40, -120, 0)
            csmNumSplits: 0
            shadowMapQuality: Light.ShadowMapQualityHigh
            softShadowQuality: Light.Hard
        }

        Model {
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
                    addConesCylindersTriple(this, z_pos)
                    z_pos -= 450
                }
            }
        }
    }

    View3D {
        width: parent.width/2
        height: parent.height
        x: offsets[1]

        camera: camera1
        environment: environment1
        DirectionalLight {
            castsShadow: true
            shadowFactor: 100
            eulerRotation: Qt.vector3d(-40, -120, 0)
            csmNumSplits: 0
            lockShadowmapTexels: true
            shadowMapQuality: Light.ShadowMapQualityHigh
            softShadowQuality: Light.Hard
        }

        Model {
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
                    addConesCylindersTriple(this, z_pos)
                    z_pos -= 450
                }
            }
        }
    }
}
