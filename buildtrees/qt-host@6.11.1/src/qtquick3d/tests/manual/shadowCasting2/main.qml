// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 640
    height: 480
    visible: true

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 300)
            clipFar: 5000
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: lightCastsShadow.checked
            brightness: 1
            shadowFactor: 100
        }

        Model {
            eulerRotation: Qt.vector3d(-45, 0, 0)
            source: "#Rectangle"
            scale: Qt.vector3d(2, 2, 2)
            materials: DefaultMaterial {
                diffuseColor: "green"
            }
            castsShadows: rectangleCastsShadows.checked
            receivesShadows: rectangleReceivesShadows.checked
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(1, 1, 1)
            materials: DefaultMaterial {
                diffuseColor: "blue"
            }
            castsShadows: cubeCastsShadows.checked
            receivesShadows: cubeReceivesShadows.checked
        }
    }

    WasdController {
        speed: 2
        controlledObject: camera
        Keys.onPressed: event => {
                            handleKeyPress(event)
                        }
        Keys.onReleased: event => {
                             handleKeyRelease(event)
                         }
    }

    Frame {
        background: Rectangle {
            color: "#c0c0c0"
            border.color: "#202020"
        }
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10

        ColumnLayout {
            CheckBox {
                id: lightCastsShadow
                checked: true
                text: qsTr("light castsShadow")
            }
            CheckBox {
                id: cubeCastsShadows
                checked: true
                text: qsTr("cube castsShadows")
            }
            CheckBox {
                id: cubeReceivesShadows
                checked: true
                text: qsTr("cube receivesShadows")
            }
            CheckBox {
                id: rectangleCastsShadows
                checked: true
                text: qsTr("rectangle castsShadows")
            }
            CheckBox {
                id: rectangleReceivesShadows
                checked: true
                text: qsTr("rectangle receivesShadows")
            }
        }
    }
}
