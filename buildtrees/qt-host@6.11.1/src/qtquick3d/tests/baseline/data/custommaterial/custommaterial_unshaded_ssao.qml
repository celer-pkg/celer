// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            aoStrength: 90
            aoDistance: 80
            aoSoftness: 50
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        Node {
            id: barrel
            position: Qt.vector3d(-292.216, -304.023, -434)
            rotation: Quaternion.fromEulerAngles(0, 0, -41.5)
            scale: Qt.vector3d(10, 10, 10)
            Model {
                rotation: Quaternion.fromEulerAngles(-90, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                source: "../shared/models/barrel/meshes/Barrel.mesh"
                materials: [ CustomMaterial {
                        shadingMode: CustomMaterial.Unshaded
                        vertexShader: "custom_unshaded_ssao.vert"
                        fragmentShader: "custom_unshaded_ssao.frag"
                    } ]
            }
        }
    }
}
