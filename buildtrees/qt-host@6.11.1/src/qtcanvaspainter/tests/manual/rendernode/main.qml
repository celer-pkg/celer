// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import TestApp

Item {
    width: 1280
    height: 720

    Item {
        id: stuff2D
        anchors.fill: parent

        Rectangle {
            id: rect
            color: "transparent"
            border.color: "white"
            border.width: 4
            width: 300
            height: 300
            clip: cbQuickClip.checked

            x: parent.width / 2 - width / 2
            y: parent.height / 2 - height / 2

            SequentialAnimation on y {
                running: cbMove.checked
                loops: -1
                NumberAnimation {
                    from: rect.parent.height / 2 - rect.height / 2
                    to: 0
                    duration: 4000
                }
                NumberAnimation {
                    from: 0
                    to: rect.parent.height / 2 - rect.height / 2
                    duration: 4000
                }
            }

            NumberAnimation on rotation {
                from: 0
                to: 360
                duration: 4000
                running: cbRotate.checked
                loops: -1
            }

            Text {
                color: "white"
                text: "Item bounds"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.topMargin: -20
            }

            RhiItem {
                id: mainRhiItem
                anchors.fill: parent

                SequentialAnimation on circleRadius {
                    running: cbGrow.checked
                    loops: -1
                    NumberAnimation {
                        from: 100
                        to: 400
                        duration: 4000
                    }
                    NumberAnimation {
                        from: 400
                        to: 100
                        duration: 4000
                    }
                }

                z: cbZ.checked ? -1 : 0

                useClipRect: cbClipRect.checked
            }
        }

        SeeThroughCircles {
            visible: cbShowSemiTransparent.checked
            width: 400
            height: 400
            anchors.centerIn: parent
            NumberAnimation on rotation {
                from: 0; to: 360; easing.type: Easing.InOutQuad; duration: 2000; loops: -1
            }
        }

        ColumnLayout {
            RowLayout {
                CheckBox {
                    id: cbMove
                    text: "Move!"
                    checked: false
                }
                CheckBox {
                    id: cbRotate
                    text: "Rotate!"
                    checked: false
                }
                CheckBox {
                    id: cbGrow
                    text: "Grow! (It's boundless!)"
                }
                CheckBox {
                    id: cbZ
                    text: "Stack below parent! (do Grow first; proves depth testing works)"
                    checked: false
                }
                CheckBox {
                    id: cbClipRect
                    text: "Use setClipRect! (limited)"
                    checked: false
                }
                CheckBox {
                    id: cbQuickClip
                    text: "Set clip: true on parent! (limited)"
                    checked: false
                }
            }
            RowLayout {
                CheckBox {
                    id: cbShowSemiTransparent
                    text: "Show a semi-transparent item on top"
                    checked: false
                }
            }
            Button {
                text: "Reset"
                onClicked: {
                    rect.y = rect.parent.height / 2 - rect.height / 2
                    rect.rotation = 0
                    mainRhiItem.circleRadius = 100
                }
            }
            Button {
                text: "I want 3D!"
                onClicked: {
                    stuff2D.visible = false
                    stuff3D.visible = true
                }
            }
        }

        RhiItem {
            anchors.bottom: parent.bottom
            width: 200
            height: 200
        }

        RhiItem {
            anchors.right: parent.right
            width: 200
            height: 200
        }

        RhiItem {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 200
            height: 200
        }
    }

    View3D {
        id: stuff3D
        visible: false
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#808080"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            property real cameraAnimation: 1
            SequentialAnimation {
                loops: Animation.Infinite
                running: true
                NumberAnimation {
                    target: camera
                    property: "cameraAnimation"
                    to: -1
                    duration: 5000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: camera
                    property: "cameraAnimation"
                    to: 1
                    duration: 5000
                    easing.type: Easing.InOutQuad
                }
            }
            position: Qt.vector3d(200 * cameraAnimation, 300, 500)
            eulerRotation.x: -20
            eulerRotation.y: 20 * cameraAnimation
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-135, -110, 0)
            brightness: 1
        }

        SpotLight {
            position: Qt.vector3d(0, 500, 600)
            eulerRotation.x: -45
            brightness: 30
        }

        Model {
            source: "#Rectangle"
            y: -500
            scale: Qt.vector3d(12, 12, 12)
            eulerRotation.x: -90
            materials: PrincipledMaterial {
                baseColor: Qt.rgba(0.5, 0.5, 0.5, 1.0)
            }
        }
        Model {
            source: "#Rectangle"
            x: -600
            scale: Qt.vector3d(12, 10, 12)
            eulerRotation.y: 90
            materials: PrincipledMaterial {
                baseColor: Qt.rgba(0.8, 0.8, 0.6, 1.0)
            }
        }
        Model {
            source: "#Rectangle"
            z: -600
            scale: Qt.vector3d(12, 10, 12)
            materials: PrincipledMaterial {
                baseColor: Qt.rgba(0.8, 0.8, 0.6, 1.0)
            }
        }

        Node {
            position: Qt.vector3d(0, 100, -120)
            RhiItem {
                width: 400
                height: 400
                anchors.centerIn: parent
                circleRadius: 200
                SequentialAnimation on circleRadius {
                    running: cb3DGrow.checked
                    loops: -1
                    NumberAnimation {
                        from: 200
                        to: 500
                        duration: 4000
                    }
                    NumberAnimation {
                        from: 500
                        to: 200
                        duration: 4000
                    }
                }
            }
        }

        Node {
            position: Qt.vector3d(0, 150, 100)
            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation {
                    to: -200
                    duration: 1500
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 200
                    duration: 1500
                    easing.type: Easing.InOutQuad
                }
            }
            NumberAnimation on eulerRotation.z {
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 4000
                easing.type: Easing.InOutBack
            }
            SeeThroughCircles {
                width: 400
                height: 400
                anchors.centerIn: parent
            }
        }

        Node {
            position: Qt.vector3d(0, 80, 250)
            Text {
                anchors.centerIn: parent
                width: 300
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
                font.pixelSize: 14
                color: "#e0e0e0"
                style: Text.Raised
                text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod " +
                        "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim " +
                        "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea " +
                        "commodo consequat."
                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 0
                        duration: 1500
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        to: 1
                        duration: 1500
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        Node {
            position: Qt.vector3d(0, 800, 0)
            SequentialAnimation on eulerRotation.x {
                loops: Animation.Infinite
                NumberAnimation {
                    to: 20
                    duration: 3500
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: -20
                    duration: 3500
                    easing.type: Easing.InOutQuad
                }
            }
            Model {
                source: "#Cylinder"
                y: -300
                scale: Qt.vector3d(0.1, 6.1, 0.1)
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.9, 0.9, 0.9, 1.0)
                }
            }
            Model {
                source: "#Sphere"
                y: -700
                scale: Qt.vector3d(2, 2, 2)
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                }
            }
        }

        ColumnLayout {
            Label {
                text: "Depth testing hopefully works."
                color: "black"
            }
            Label {
                text: "No clipping support in this mode."
                color: "black"
            }
            CheckBox {
                id: cb3DGrow
                text: "Grow!"
                checked: false
            }
            Button {
                text: "2D is enough"
                onClicked: {
                    stuff3D.visible = false
                    stuff2D.visible = true
                }
            }
        }
    }
}
