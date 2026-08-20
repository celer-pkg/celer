// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: opacitymap
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0
        width: parent.width * 1
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
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
            id: rectangle
            position: Qt.vector3d(15.6206, 1.91976, 21.2664)
            rotation: Quaternion.fromEulerAngles(-62.5, 0, 0)
            scale: Qt.vector3d(6.24243, 4.98461, 1)
            source: "#Rectangle"
            
            

            DefaultMaterial {
                id: material
                lighting: DefaultMaterial.FragmentLighting
                diffuseColor: Qt.rgba(1, 0.870588, 0.752941, 1)
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [material]
        }

        Model {
            id: sphere
            position: Qt.vector3d(3.93619, 42.917, 251.294)
            source: "#Sphere"
            
            

            DefaultMaterial {
                id: material_001
                lighting: DefaultMaterial.FragmentLighting
                diffuseColor: Qt.rgba(0, 0.878431, 0, 1)
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                opacityMap: material_001_opacitymap
                bumpAmount: 0.5
                translucentFalloff: 1

                Texture {
                    id: material_001_opacitymap
                    source: "../shared/maps/opacitymap.png"
                }
            }
            materials: [material_001]
        }

        Model {
            id: cube
            position: Qt.vector3d(-259.951, 176.081, 5.02271)
            rotation: Quaternion.fromEulerAngles(30.5, 34, 0)
            source: "#Cube"
            
            

            DefaultMaterial {
                id: material_002
                lighting: DefaultMaterial.FragmentLighting
                diffuseColor: Qt.rgba(1, 0.658824, 0, 1)
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                opacityMap: material_002_opacitymap
                bumpAmount: 0.5
                translucentFalloff: 1

                Texture {
                    id: material_002_opacitymap
                    source: "../shared/maps/opacitymap.png"
                }
            }
            materials: [material_002]
        }
    }
}
