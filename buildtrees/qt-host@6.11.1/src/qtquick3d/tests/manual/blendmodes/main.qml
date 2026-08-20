// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick3D

Window {
    visible: true
    width: 1280
    height: 720
    title: qsTr("Blend Modes Example")
    color: "#6b7080"

    Column {
        id: controlArea
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 20
        width: 220

        Text {
            text: qsTr("Background model")
            font.pixelSize: 20
            font.bold: true
        }
        ListView {
            id: modeList
            width: parent.width
            height: childrenRect.height
            model: modeModel
            delegate: Item {
                height: 26
                width: 140
                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    text: mode
                    font.pixelSize: 20
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: modeList.currentIndex = index;
                }
            }
            highlight: Rectangle {
                color: "#53586b"
                radius: 4
            }
            focus: true
        }
        Item {
            width: 1
            height: 20
        }

        Text {
            text: qsTr("Foreground model")
            font.pixelSize: 20
            font.bold: true
        }
        ListView {
            id: modeList2
            width: parent.width
            height: childrenRect.height
            model: modeModel
            delegate: Item {
                height: 26
                width: 140
                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    text: mode
                    font.pixelSize: 20
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: modeList2.currentIndex = index;
                }
            }
            highlight: Rectangle {
                color: "#53586b"
                radius: 4
            }
            focus: true
        }

    }

    ListModel {
        id: modeModel
        ListElement {
            mode: "SourceOver"
            modeType: DefaultMaterial.SourceOver
        }
        ListElement {
            mode: "Screen"
            modeType: DefaultMaterial.Screen
        }
        ListElement {
            mode: "Multiply"
            modeType: DefaultMaterial.Multiply
        }
    }

    Item {
        id: viewArea
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: controlArea.right

        View3D {
            anchors.fill: parent
            environment: SceneEnvironment {
                clearColor: "#848895"
                backgroundMode: SceneEnvironment.Color
            }

            DirectionalLight {
                brightness: 2
            }

            PerspectiveCamera {
                z: 500
            }

            // Background model
            Model {
                source: "#Cube"
                rotation: Quaternion.fromEulerAngles(-45, -45, 22.5)
                scale: Qt.vector3d(2,2,2)
                materials: DefaultMaterial {
                    diffuseColor: "#a8171a"
                    blendMode: modeModel.get(modeList.currentIndex).modeType
                }
            }

            // Foreground model
            Model {
                source: "#Cone"
                scale: Qt.vector3d(3,3,3)
                position.z: 50
                position.y: -100
                materials: DefaultMaterial {
                    diffuseColor: "#17a81a"
                    blendMode: modeModel.get(modeList2.currentIndex).modeType
                }
            }
        }
    }
}
