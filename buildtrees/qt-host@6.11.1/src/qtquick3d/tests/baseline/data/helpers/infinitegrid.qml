// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color

            InfiniteGrid {
                gridInterval: 10
            }
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 300, 600)
            eulerRotation.x: -45
        }

        DirectionalLight {
        }

        Model {
            source: "#Cube"
            eulerRotation.y: 30
            materials: PrincipledMaterial { baseColor: "#337700" }
        }
    }
}
