// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    property vector3d qmlxr_originPosition: Qt.vector3d(0, 0, 600)

    property SceneEnvironment qmlxr_environment: SceneEnvironment {
        depthPrePassEnabled: true
    }

    DirectionalLight {
    }

    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseColor: "red"
        }
        NumberAnimation  on eulerRotation.y {
            duration: 10000
            easing.type: Easing.InOutQuad
            from: 0
            to: 360
            running: true
            loops: -1
        }
    }

    Node {
        y: -50
        Text {
            text: "hello world"
            font.pointSize: 32
            color: "white"
        }
    }
}
