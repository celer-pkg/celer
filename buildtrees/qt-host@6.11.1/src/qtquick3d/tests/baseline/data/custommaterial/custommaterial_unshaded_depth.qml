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

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation: Qt.vector3d(-30, 0, 0)
        }

        Model {
            eulerRotation: Qt.vector3d(0, 120, 340)
            source: "../shared/models/monkey_object.mesh"
            scale: Qt.vector3d(80, 80, 80)
            materials: [ CustomMaterial {
                    cullMode: Material.NoCulling
                    shadingMode: CustomMaterial.Unshaded
                    vertexShader: "custom_unshaded_depth.vert"
                    fragmentShader: "custom_unshaded_depth.frag"
                } ]
        }
    }
}
