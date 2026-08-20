// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: scopedLights
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
            color: Qt.rgba(0.486275, 0.992157, 1, 1)
            shadowFactor: 10
            shadowFilter: 36.97999954223633
            scope: cube
        }

        Model {
            id: cube
            position: Qt.vector3d(-284.867, 0, 0)
            source: "#Cube"
            
            

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
            id: cube2
            position: Qt.vector3d(340.036, 0, 0)
            source: "#Cube"
            
            

            DefaultMaterial {
                id: default_001
                lighting: DefaultMaterial.FragmentLighting
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
            }
            materials: [default_001]
        }

        DirectionalLight {
            id: light2
            color: Qt.rgba(0.682353, 0.682353, 1, 1)
            shadowFactor: 10
            scope: cube2
        }
    }

    View3D {
        id: layer2
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
            id: camera_001
            position: Qt.vector3d(0, 0, 600)
            clipFar: 5000
        }

        Model {
            id: cube_001
            position: Qt.vector3d(0, 169.409, 0)
            source: "#Cube"
            
            

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
    }
}
