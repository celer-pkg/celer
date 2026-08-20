// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Model {
    source: "meshes/block.mesh"
    property real stoneSize : 1
    property real stoneRotationY : 0
    property real stoneRotationZ : 0
    scale: Qt.vector3d(stoneSize * 15, stoneSize * 15, stoneSize * 15)
    eulerRotation: Qt.vector3d(90, stoneRotationY, stoneRotationZ)
    materials: [
        PrincipledMaterial {
            baseColorMap: Texture {
                source: "images/firestone.png"
            }
        }
    ]
}
