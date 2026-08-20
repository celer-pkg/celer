// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: directionallight
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
            position: Qt.vector3d(-5.77344, -34.641, -0.5)
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

        Model {
            id: cylinder
            position: Qt.vector3d(-28.4985, 11.4019, 70.381)
            source: "#Cylinder"
            
            

            DefaultMaterial {
                id: material_001
                lighting: DefaultMaterial.FragmentLighting
                diffuseColor: Qt.rgba(0.501961, 1, 0.501961, 1)
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [material_001]
        }

        DirectionalLight {
            id: shadowcaster_area
            rotation: Quaternion.fromEulerAngles(-46.101, -128.424, 60.9979)
            castsShadow: true
            shadowFactor: 100
        }
    }
}
