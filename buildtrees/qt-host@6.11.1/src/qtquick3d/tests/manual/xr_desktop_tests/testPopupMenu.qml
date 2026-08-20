// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Xr
import QtQuick3D.Helpers

pragma ComponentBehavior: Bound
Window {
    id: window
    width: 1280
    height: 720
    visible: true

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "skyblue"
            backgroundMode: SceneEnvironment.Color
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData{}
            }
            InfiniteGrid {
                visible: true
                gridInterval: 1
            }
        }

        Node {
            id: cameraOrigin
            y: 50
            eulerRotation.x: -5
            eulerRotation.y: 15
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 150)
            }
        }
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            ambientColor: "#777"
        }

        XrItem {
            id: theItem
            width: 160
            height: 100
            x: -width/2
            y: height
            z: 40
            color: "transparent"

            property int count: 1
            contentItem: Rectangle {
                color: Qt.rgba(1, 1, 1, 0.5)
                border.width: 5
                border.color: "lightblue"
                height: 300
                width: 160 * 3
                radius: 25

                Repeater {
                    model: theItem.count
                    Rectangle {
                        required property int index
                        x: 100 + 50 * index
                        y: 100
                        width: 30
                        height: 30
                        rotation: 45
                        color: "red"
                    }
                }

                Button {
                    anchors.centerIn: parent
                    id: fileButton
                    text: "Open menu"
                    onClicked: menu.open()

                    Menu {
                        id: menu
                        y: fileButton.height
                        popupType: Popup.Item
                        MenuItem {
                            text: "One"
                            onTriggered: theItem.count = 1
                        }
                        MenuItem {
                            text: "Two"
                            onTriggered: theItem.count = 2
                        }
                        MenuItem {
                            text: "Three"
                            onTriggered: theItem.count = 3
                        }
                    }
                }

                MenuBar {
                    width: parent.width
                    Menu {
                        title: qsTr("&File")
                        Action { text: qsTr("&New...") }
                        Action { text: qsTr("&Open...") }
                        Action { text: qsTr("&Save") }
                        Action { text: qsTr("Save &As...") }
                        MenuSeparator { }
                        Action {
                            text: qsTr("&Quit")
                            onTriggered: Qt.quit()
                        }
                    }
                    Menu {
                        title: qsTr("&Edit")
                        Action { text: qsTr("Cu&t") }
                        Action { text: qsTr("&Copy") }
                        Action { text: qsTr("&Paste") }
                    }
                    Menu {
                        title: qsTr("&Help")
                        Action { text: qsTr("&About") }
                    }
                }
            }
        }
    }

    OrbitCameraController {
        id: orbitController
        acceptedButtons:  Qt.MiddleButton
        origin: cameraOrigin
        camera: camera
    }
}
