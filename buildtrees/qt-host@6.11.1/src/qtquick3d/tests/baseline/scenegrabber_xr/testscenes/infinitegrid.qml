// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 300, 600)
    property vector3d qmlxr_originRotation: Qt.vector3d(-45, 0, 0)
    property SceneEnvironment qmlxr_environment: SceneEnvironment {
        clearColor: "black"
        backgroundMode: SceneEnvironment.Color

        InfiniteGrid {
            gridInterval: 10
        }
    }

    DirectionalLight {
    }

    Model {
        source: "#Cube"
        eulerRotation.y: 30
        materials: PrincipledMaterial { baseColor: "#337700" }
    }
}
