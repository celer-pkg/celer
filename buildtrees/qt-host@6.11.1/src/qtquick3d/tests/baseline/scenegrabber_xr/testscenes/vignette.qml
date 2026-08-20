// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 0, 100)
    property SceneEnvironment qmlxr_environment: ExtendedSceneEnvironment {
        backgroundMode: SceneEnvironment.Color
        clearColor: "lightGray"
        vignetteEnabled: true
        vignetteStrength: 15
        vignetteRadius: 0.45
    }

    DirectionalLight {
    }

    Model {
        source: "#Sphere"
        materials: [ PrincipledMaterial {
                id: sphereMaterial
                baseColor: "green"
                metalness: 0.5
            } ]
    }
}
