// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

View3D {
    width: 300
    height: 300

    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.Transparent
    }

    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 600)
    }

    DirectionalLight {
    }

    Model {
        source: "#Cube"
        materials: PrincipledMaterial { baseColor: "green" }
        NumberAnimation on eulerRotation.y {
            from: 0; to: 360; duration: 3000; loops: -1
        }
    }
}
