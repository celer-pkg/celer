// Copyright (C) 2022 The Qt Company Ltd.
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
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#434343"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 10, 10)
            eulerRotation: Qt.vector3d(-45, 0, 0)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.4, 0.2, 0.6, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        Model {
            id: hfModel
            geometry: HeightFieldGeometry {
                extents: Qt.vector3d(10, 2, 5)
                heightMap: "heightfield.png"
            }
            materials: PrincipledMaterial {
                baseColor: "#ff77ff"
                roughness: 0.3
            }
            opacity: 0.8
        }
    }
}
