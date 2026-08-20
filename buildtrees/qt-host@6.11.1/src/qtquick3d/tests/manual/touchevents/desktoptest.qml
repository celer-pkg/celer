// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D
import QtQuick3D.Xr
import QtQuick.Controls

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    Item {
        property bool touched: false

        focus: true
        Keys.onPressed: (ev) => {
            switch (ev.key) {
                case Qt.Key_Space:
                touched = !touched
                view.setTouchpoint(theItem2D, Qt.point(350, 100), 3, touched)
                break
            }
        }
    }

    component TouchButton : Button {
        required property point pos
        required property int touchId
        checkable: true

        property bool touched: false

        font.bold: touched

        function toggleTouch() {
            touched = !touched
            view.setTouchpoint(theItem2D, pos, touchId, touched)
        }

        onCheckedChanged: {
            if (!checked && touched)
                toggleTouch()
        }

        Timer {
            interval: 500
            repeat: true
            running: parent.checked
            onTriggered: parent.toggleTouch()
        }
    }

    component TouchClickButton : Button {
        id: tcbRoot
        required property point pos
        required property int touchId

        property bool touched: false
        font.bold: touched


        property point currentPoint: Qt.point(pos.x + offset, pos.y)
        function sendEvent() {
            view.setTouchpoint(theItem2D, currentPoint, touchId, touched)
        }

        property real offset: 0
        onOffsetChanged: sendEvent()
        onPressed: {
            clickAction.start()
        }

        SequentialAnimation on offset {
            id: clickAction
            running: false
            ScriptAction {
                script: {
                    tcbRoot.offset = 0
                    tcbRoot.touched = true
                    tcbRoot.sendEvent()
                }
            }
            PropertyAnimation {
                from: 0
                to: 5
                duration: 400
            }
            ScriptAction {
                script: {
                    tcbRoot.touched = false
                    tcbRoot.sendEvent()
                }
            }
        }
    }

    component TouchDoubleClickButton : Button {
        id: tdcbRoot
        required property point pos
        required property int touchId
        property alias loops: doubleClickAction.loops
        property alias duration: clickPause.duration

        property bool touched: false
        font.bold: touched

        function sendEvent() {
            view.setTouchpoint(theItem2D, pos, touchId, touched)
        }

        onClicked: {
            doubleClickAction.start()
        }

        SequentialAnimation {
            id: doubleClickAction
            running: false
            loops: 4
            ScriptAction {
                script: {
                    tdcbRoot.touched = !tdcbRoot.touched
                    tdcbRoot.sendEvent()
                }
            }
            PauseAnimation {
                id: clickPause
                duration: 100
            }
        }
    }

    component TouchLongPressButton : TouchDoubleClickButton {
        loops: 2
        duration: 1000
    }

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "skyblue"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 150)
        }

        Node {
            scale: "0.2,0.2,0.2"
            y: 50
            Rectangle {
                id: theItem2D
                width: 500
                height: 500
                color: "#ffcc77"

                Rectangle {
                    id: mouseRect

                    width: 200
                    height: 200

                    color: mouser.pressed ? "red" : "green"

                    Rectangle {
                        width: 30
                        height: 30
                        radius: 15
                        color: "white"
                        x: mouser.mouseX - 15
                        y: mouser.mouseY - 15
                    }

                    MouseArea {
                        id: mouser
                        anchors.fill: parent
                        onClicked: logger.log("Click on MouseArea")
                    }
                }

                Rectangle {
                    id: tapRect
                    x: 250
                    width: 200
                    height: 200

                    color: tapper.pressed ? "lightblue" : "darkblue"

                    Text {
                        text: tapper.count + ", " + tapper.tapCount
                        color: "white"
                        font.pixelSize: 30
                    }

                    TapHandler {
                        id: tapper
                        objectName: "myTapHandler"

                        // dragThreshold: 75

                        property int count: 0
                        onTapped: count++

                        onSingleTapped: logger.log("Single tap on TapHandler")
                        onDoubleTapped: logger.log("Double tap on TapHandler")
                        onLongPressed: logger.log("LONG press on TapHandler")
                    }
                }

                Rectangle {
                    id: touchRect
                    y: 250
                    width: 200
                    height: 200

                    color: toucher.isPressed ? "orange" : "brown"

                    MultiPointTouchArea {
                        id: toucher
                        anchors.fill: parent

                        property bool isPressed: false
                        onPressed: {
                            isPressed = true // simplistic...
                            logger.log("Press on MultiPointTouchArea")
                        }
                        onReleased: {
                            isPressed = false
                            logger.log("Release on MultiPointTouchArea")
                        }
                    }
                }
            }
        }
    } // View3D

    Column {
        id: buttonColumn
        x: 10
        y: 10
        TouchButton {
            text: "Touch on MouseArea"
            touchId: 5
            pos: Qt.point(100, 100)
        }
        TouchButton {
            text: "Touch on TapHandler"
            touchId: 6
            pos: Qt.point(350, 100)
        }
        TouchButton {
            text: "Touch on MultiPointTouchArea"
            touchId: 8
            pos: Qt.point(100, 350)
        }

        TouchClickButton {
            text: "Touch drag on Mouse Area"
            touchId: 1
            pos: Qt.point(100, 100)
        }
        TouchClickButton {
            text: "Touch drag on TapHandler"
            touchId: 6
            pos: Qt.point(350, 100)
        }
        TouchDoubleClickButton {
            text: "Touch double click on TapHandler"
            touchId: 6
            pos: Qt.point(350, 100)
        }
        TouchLongPressButton {
            text: "Touch long press on TapHandler"
            touchId: 6
            pos: Qt.point(350, 100)
        }
        Rectangle {
            width: 200
            height: 200
            Text {
                id: logger
                function log(str: string) {
                    console.log(str)
                    if (text.length > 200)
                        text = ""
                    text += str + "\n"
                }

                anchors.fill: parent
                wrapMode: Text.WordWrap
            }
        }
    }
}
