// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

Rectangle {
    id: window
    width: 640
    height: 480
    visible: true
    color: "black"

    Blur { id: blur0 }
    Blur { id: blur1 }
    Blur { id: blur2 }
    Blur { id: blur3 }

    View3D {
        width: parent.width/2
        height: parent.height

        environment: SceneEnvironment {
            effects: [blur1, blur2, blur3]
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
        }

        DirectionalLight {
            eulerRotation.x: -30
        }

        Model {
            source: "#Cube"
            materials: DefaultMaterial {}
        }
    }

    View3D {
        x: parent.width/2
        width: parent.width/2
        height: parent.height

        environment: SceneEnvironment {
            effects: [blur0, blur0, blur0]
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
        }

        DirectionalLight {
            eulerRotation.x: -30
        }

        Model {
            source: "#Cube"
            materials: DefaultMaterial {}
        }
    }
}
