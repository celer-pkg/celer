// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

View3D {
    id: viewport
    environment: SceneEnvironment {
        clearColor: "#d6dbdf"
        backgroundMode: SceneEnvironment.Color
    }
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 1000)
        clipFar: 2000
        clipNear: 1
    }
    DirectionalLight {
        eulerRotation.x: -45
        eulerRotation.y: 45
        castsShadow: false
        brightness: 1
        shadowFactor: 100
    }
}
