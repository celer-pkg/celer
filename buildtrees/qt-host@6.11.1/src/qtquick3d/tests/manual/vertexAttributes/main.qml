// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick3D
import QtQuick3D.Helpers
import Example 1.0

Window {
    id: window
    visible: true
    width: 1600
    height: 1020
    title: qsTr("Vertex Attributes")

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "white"
            backgroundMode: SceneEnvironment.Color
        }
        PerspectiveCamera {
            id: camera
            clipFar: 3000
        }
        PointLight {
            color: Qt.rgba(0.9, 0.8, 0.7, 1.0)
            quadraticFade: 0.0
            linearFade: 0.01
        }
        Exhibit {
            position: true
            text: "Position Only"
        }
        Exhibit {
            position: true
            normal: true
            eulerRotation.y: 60
            text: "+ Normal"
        }
        Exhibit {
            position: true
            normal: true
            texcoord0: true
            eulerRotation.y: 120
            text: "+ Texcoord"
        }
        Exhibit {
            position: true
            normal: true
            texcoord0: true
            tangent: true
            eulerRotation.y: 180
            text: "+ Tangent"
        }
        Exhibit {
            position: true
            normal: true
            texcoord0: true
            tangent: true
            binormal: true
            eulerRotation.y: 240
            text: "+ Binormal"
        }
        Exhibit {
            position: true
            normal: true
            texcoord0: true
            binormal: true
            tangent: true
            color: true
            eulerRotation.y: 300
            text: "+ Color"
        }
        WasdController {
            id: wasdController
            controlledObject: camera
        }
    }
}
