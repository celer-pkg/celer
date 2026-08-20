// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(1, 1, 1, 1)

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#556677"
            tonemapMode: SceneEnvironment.TonemapModeHejlDawson
        }

        PerspectiveCamera {
            position.z: 600
        }

        DirectionalLight {
        }

        TonemappingTestScene {
        }
    }
}
