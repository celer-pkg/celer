// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick3D.Helpers

Window {
    id: mainWindow
    width: 1280
    height: 720
    visible: true

    property real fontSize: 14

    Text {
        x : mainWindow.width / 4
        text: "Fixed length"
        font.pixelSize: mainWindow.fontSize
        color: "black"
    }
    Text {
        id: textbox
        x : 3 * mainWindow.width / 4
        text: "Variable length"
        font.pixelSize: mainWindow.fontSize
        color: "black"
    }

    View3D {
        anchors.topMargin: textbox.height
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#101010"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.VeryHigh
        }
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 600)
            clipFar: 2000
        }

        Model {
            source: "#Cube"
            position: Qt.vector3d(-400, -50, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.001)
            materials: [
                DefaultMaterial {
                    diffuseColor: "white"
                    lighting: DefaultMaterial.NoLighting
                }
            ]
        }

        LineEmitterSystem {
            id: fixedSize
            position: Qt.vector3d(-500, 0, 0)
        }

        LineEmitterSystem {
            id: variableSize
            position: Qt.vector3d(100, 0, 0)
            gravityStrength: 150.0
            variableLengthEnabled: true
            lifespan: 2000
        }
    }
}
