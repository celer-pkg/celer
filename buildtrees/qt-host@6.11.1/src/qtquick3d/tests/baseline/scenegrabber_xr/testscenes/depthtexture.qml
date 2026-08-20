// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 200, 300)
    property vector3d qmlxr_originRotation: Qt.vector3d(-30, 0, 0)
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(3, 3, 3)
        materials: [ CustomMaterial {
                cullMode: Material.NoCulling
                shadingMode: CustomMaterial.Unshaded
                vertexShader: "custom_unshaded_depth.vert"
                fragmentShader: "custom_unshaded_depth.frag"
            } ]
    }
}
