// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: progressive_aa
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
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
            antialiasingMode: SceneEnvironment.ProgressiveAA
            antialiasingQuality: SceneEnvironment.VeryHigh
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
            id: cube
            position: Qt.vector3d(0, 0, 0)
            rotation: Quaternion.fromEulerAngles(28.0299, 33.3145, 17.1637)
            scale: Qt.vector3d(4.00606, 2, 2)
            source: "#Cube"

            Vector3dAnimation on eulerRotation {
                duration: 200
                from: Qt.vector3d(28.0299, 33.3145, 17.1637)
                to: Qt.vector3d(170.0299, 58.3145, 96.1637)
            }

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
    }
}
