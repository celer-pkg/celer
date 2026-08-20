// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: image_params_control
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
            position: Qt.vector3d(-280.015, -1.5, 0)
            scale: Qt.vector3d(5, 5, 1)
            source: "#Rectangle"
            
            

            DefaultMaterial {
                id: material
                lighting: DefaultMaterial.FragmentLighting
                diffuseMap: material_diffusemap
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1

                Texture {
                    id: material_diffusemap
                    source: "../shared/maps/oulu_2.jpeg"
                    tilingModeHorizontal: Texture.Repeat
                }
            }
            materials: [material]
        }

        Model {
            id: rectangle2
            position: Qt.vector3d(280.015, -1.5, 0)
            scale: Qt.vector3d(5, 5, 1)
            source: "#Rectangle"
            
            

            DefaultMaterial {
                id: material_001
                lighting: DefaultMaterial.FragmentLighting
                diffuseMap: material_001_diffusemap
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1

                Texture {
                    id: material_001_diffusemap
                    source: "../shared/maps/oulu_2.jpeg"
                    tilingModeHorizontal: Texture.MirroredRepeat
                    tilingModeVertical: Texture.MirroredRepeat
                    positionV: 0.20000000298023224
                    pivotU: -0.5
                    pivotV: -0.20000000298023224
                }
            }
            materials: [material_001]
        }
    }
}
