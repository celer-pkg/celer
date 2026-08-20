// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 200, 300)
    property vector3d qmlxr_originRotation: Qt.vector3d(-30, 0, 0)
    property SceneEnvironment qmlxr_environment: SceneEnvironment {
        aoStrength: 75
        aoBias: 0.5
        aoSampleRate: 2
        NumberAnimation on aoDistance { from: 10; to: 100; duration: 2000; loops: -1 }
        aoSoftness: 15
        aoDither: true
    }

    DirectionalLight {
    }

    Model {
        source: "models/object1.mesh"
        scale: Qt.vector3d(80, 80, 80)
        materials: PrincipledMaterial { baseColor: "white" }
        eulerRotation: Qt.vector3d(0, 120, 340)
    }
}
