// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

import "../shared/"

Rectangle {
    id: geometry_4
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer2
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.4
        width: parent.width * 1
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: Qt.rgba(0.501961, 1, 0.501961, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        PerspectiveCamera {
            id: camera_001
            position: Qt.vector3d(0, 0, 600)
            clipFar: 5000
        }

        DirectionalLight {
            id: light_001
            shadowFactor: 10
        }

        Model {
            id: sphere
            position: Qt.vector3d(12.67, 168.035, -34.9131)
            source: "#Sphere"
            
            

            DefaultMaterial {
                id: material_001
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [material_001]
        }
    }

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
            position: Qt.vector3d(-5.77344, -34.641, 0)
            rotation: Quaternion.fromEulerAngles(-53.5, 0, 0)
            scale: Qt.vector3d(6.30691, 5.36799, 1)
            source: "#Rectangle"
            
            

            DefaultMaterial {
                id: material
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [material]
        }
    }
}
