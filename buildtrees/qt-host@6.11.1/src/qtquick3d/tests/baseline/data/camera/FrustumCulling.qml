// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick
import QtQuick3D.Lancelot

Rectangle {
    id: perspective_camera
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    Node {
        id: scene

        DirectionalLight {
            id: light
            shadowFactor: 10
        }
    }

    View3D {
        id: leftView
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width * 0.5
        height: parent.height * 1
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
        }

        camera: PerspectiveCamera {
            position: Qt.vector3d(0, 0, 3)
            clipNear: 1
            clipFar: 3
            frustumCullingEnabled: false
        }

        Model {
            geometry: DynamicGeometry {
                id: geometry

            }
            materials: [
                PrincipledMaterial {
                    baseColor: "green"
                    lighting: PrincipledMaterial.NoLighting
                }
            ]
        }

        importScene: scene
    }

    View3D {
        id: rightView
        anchors.left: leftView.right
        anchors.top: parent.top
        anchors.right: parent.right
        height: parent.height * 1
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
        }

        camera: PerspectiveCamera {
            position: Qt.vector3d(0, 0, 3)
            clipNear: 1
            clipFar: 3
            frustumCullingEnabled: true
        }

        importScene: scene

        Model {
            geometry: DynamicGeometry {
                Component.onCompleted: changeBounds()

            }
            materials: [
                PrincipledMaterial {
                    baseColor: "red"
                    lighting: PrincipledMaterial.NoLighting
                }
            ]
        }

    }
}
