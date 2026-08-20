// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: ambient_occlusion
    width: 800
    height: 480
    color: Qt.rgba(1, 1, 1, 1)

    Node {
        id: scene
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 1100)
            clipFar: 5000
        }

        DirectionalLight {
            id: light
            shadowFactor: 10
        }

        Model {
            id: sphere
            position: Qt.vector3d(-354.989, 135.238, 0)
            source: "#Sphere"

            DefaultMaterial {
                id: default_
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_]
        }

        Model {
            id: cone
            position: Qt.vector3d(-365.912, -248.222, 0)
            scale: Qt.vector3d(2.89542, 3.13161, 1)
            source: "#Cone"

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
            id: cube
            position: Qt.vector3d(349.297, -228.053, 0)
            rotation: Quaternion.fromEulerAngles(28.0299, 33.3145, 17.1637)
            scale: Qt.vector3d(2.00606, 1, 1)
            source: "#Cube"

            DefaultMaterial {
                id: default_003
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_003]
        }

        Node {
            id: barrel
            position: Qt.vector3d(-292.216, -304.023, -434)
            rotation: Quaternion.fromEulerAngles(0, 0, -41.5)
            scale: Qt.vector3d(10, 10, 10)

            Model {
                id: barrel_1
                rotation: Quaternion.fromEulerAngles(-90, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                source: "../shared/models/barrel/meshes/Barrel.mesh"

                DefaultMaterial {
                    id: barrel_001
                    lighting: DefaultMaterial.FragmentLighting
                    diffuseColor: Qt.rgba(0.639994, 0.639994, 0.639994, 1)
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 9.607839584350586
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [barrel_001]
            }
        }

        Model {
            id: cylinder
            position: Qt.vector3d(255.743, -27.1591, 185)
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            source: "#Cylinder"

            DefaultMaterial {
                id: default_001
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                opacity: 0.76
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_001]
        }
    }

    View3D {
        id: leftView
        anchors.left: parent.left
        width: parent.width * 0.5
        anchors.top: parent.top
        height: parent.height
        importScene: scene
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoStrength: 25 * workaround
            aoDither: true
            aoBias: 0.5
            aoEnabled: true
            depthPrePassEnabled: true

            // The directGL code does not enable SSAO on the first couple of frames
            // This animation makes sure we get past that point. Lancelot will wait
            // for the output to stabilize.
            property int workaround: 0
            NumberAnimation on workaround {
                from: 0
                to: 3
                loops: 1
                duration: 300
            }
        }
    }

    View3D {
        id: rightView
        anchors.left: leftView.right
        width: parent.width * 0.5
        anchors.top: parent.top
        height: parent.height
        importScene: scene
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoEnabled: false
            depthPrePassEnabled: true
        }
    }
}
