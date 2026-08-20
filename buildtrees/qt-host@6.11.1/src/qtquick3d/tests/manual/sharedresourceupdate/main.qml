// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick3D

Window {
    width: 1000
    height: 800
    visible: true
    title: qsTr("Shared Resource Update tst")

    Rectangle {
        id: rectangle
        anchors.fill: parent
        color: "white"

        Button {
            id: button
            text: qsTr("Press me")
            anchors.verticalCenter: parent.verticalCenter
            checkable: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: label
            text: qsTr("Unchecked!")
            anchors.top: button.bottom
            anchors.topMargin: 45
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Item {
            id: __materialLibrary__
            DefaultMaterial {
                id: defaultMaterial
                diffuseColor: "#ff0000"
                objectName: "New Material"

                NumberAnimation on opacity {
                    running: true
                    from: 0.0
                    to: 1.0
                    duration: 3000
                    loops: -1

                }
            }
        }

        View3D {
            id: view3D
            x: 0
            y: 184
            width: parent.width / 2
            height: 400
            SceneEnvironment {
                id: sceneEnvironment
                antialiasingQuality: SceneEnvironment.High
                antialiasingMode: SceneEnvironment.MSAA
            }

            Node {
                id: scene
                DirectionalLight {
                    id: directionalLight
                }

                PerspectiveCamera {
                    id: sceneCamera
                    z: 350
                }

                Model {
                    id: cubeModel
                    source: "#Cube"
                    materials: defaultMaterial
                    eulerRotation.y: 45
                    eulerRotation.x: 30
                }
            }
            environment: sceneEnvironment
        }

        View3D {
            id: view3D1
            x: 500
            y: 201
            width: parent.width / 2
            height: 400
            SceneEnvironment {
                id: sceneEnvironment1
                antialiasingQuality: SceneEnvironment.High
                antialiasingMode: SceneEnvironment.MSAA
            }

            Node {
                id: scene1
                DirectionalLight {
                    id: directionalLight1
                }

                PerspectiveCamera {
                    id: sceneCamera1
                    z: 350
                }

                Model {
                    id: cubeModel1
                    source: "#Cube"
                    materials: defaultMaterial
                    eulerRotation.y: 45
                    eulerRotation.x: 30
                }
            }
            environment: sceneEnvironment1
        }
        states: [
            State {
                name: "clicked"
                when: button.checked

                PropertyChanges {
                    target: label
                    text: qsTr("Button Checked")
                }

                PropertyChanges {
                    target: defaultMaterial
                    diffuseColor: "#24ff00"
                }
            }
        ]
    }
}
