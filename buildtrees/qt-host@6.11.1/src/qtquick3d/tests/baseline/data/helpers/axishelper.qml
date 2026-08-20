// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick3D.Helpers
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.4, 0.2, 0.6, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        Model {
            source: "#Sphere"
            materials: [ DefaultMaterial { } ]
        }

        AxisHelper {
        }
    }
}
