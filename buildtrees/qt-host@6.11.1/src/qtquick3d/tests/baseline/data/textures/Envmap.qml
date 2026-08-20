// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: envmap
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

        PointLight {
            id: light
            position: Qt.vector3d(458.993, -407.032, 0)
            scale: Qt.vector3d(5.6848, 0.207183, 1)
            shadowFactor: 10
        }

        Model {
            id: sphere
            scale: Qt.vector3d(7.95522, 5.71875, 1)
            source: "#Sphere"
            
            

            DefaultMaterial {
                id: material
                lighting: DefaultMaterial.FragmentLighting
                specularReflectionMap: material_specularreflection
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1

                Texture {
                    id: material_specularreflection
                    source: "../shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
                    mappingMode: Texture.Environment
                }
            }
            materials: [material]
        }
    }
}
