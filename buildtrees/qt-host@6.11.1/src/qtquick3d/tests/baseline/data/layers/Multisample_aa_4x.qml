// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

import "../shared/"

Rectangle {
    id: multisample_aa_4x
    width: 800
    height: 480
    color: Qt.rgba(1, 1, 1, 1)

    View3D {
        id: layer
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0
        width: parent.width * 1
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
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
}
