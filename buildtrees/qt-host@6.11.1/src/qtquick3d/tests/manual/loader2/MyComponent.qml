// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    id: rootItem
    anchors.fill: parent

    Text {
        anchors.centerIn: parent
        z: 2
        font.pixelSize: 40
        text: "View3D here"
    }

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#600000"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        PointLight {
            position: Qt.vector3d(0, 400, 100)
            brightness: 10
        }

        Model {
            source: "#Cube"
            materials: DefaultMaterial {
            }
            eulerRotation.x: 30
            NumberAnimation on eulerRotation.z {
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 2000
                easing.type: Easing.InOutQuad
            }
        }
    }
}
