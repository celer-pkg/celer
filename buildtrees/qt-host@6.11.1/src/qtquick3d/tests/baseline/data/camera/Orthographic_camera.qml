// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: orthographic_camera
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

        OrthographicCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
            clipFar: 5000
        }

        DirectionalLight {
            id: light
            shadowFactor: 10
        }

        Model {
            id: cylinder
            position: Qt.vector3d(-165.988, -128.46, 0)
            source: "#Cylinder"
            
            

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
            id: cylinder2
            position: Qt.vector3d(55.7838, 116.872, 200)
            source: "#Cylinder"
            
            

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
}
