// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.9
import QtQuick.Window 2.2
import Qt.labs.lottieqt 1.0

Window {
    visible: true
    width: animContainer.width
    height: animContainer.height
    title: qsTr("Animation test")

    property bool showFps: false

    color: "black"

    Item {
        id: animContainer
        width: childrenRect.width
        height: childrenRect.height

        Repeater {
            model: 1

            LottieAnimation {
                id: bmAnim

                x: 10 * index
                y: 10 * index
                loops: LottieAnimation.Infinite
                quality: LottieAnimation.MediumQuality
                source: "rect_rotate.json"
            }
        }
    }

    Text {
        id: text

        property real t
        property int frame: 0

        anchors.right: parent.right
        color: "red"
        visible: showFps
        text: "FPS: " + fpsTimer.fps

        Timer {
            id: fpsTimer
            property real fps: 0
            repeat: true
            interval: 1000
            running: showFps
            onTriggered: {
                parent.text = "FPS: " + fpsTimer.fps
                fps = text.frame
                text.frame = 0
            }
        }

        NumberAnimation on t {
            id: tAnim
            from: 0
            to: 1000
            running: showFps
            loops: Animation.Infinite
        }

        onTChanged: {
            update() // force continuous animation
            text.frame++;
        }
    }

    Rectangle {
        id: rot

        width: 10
        height: 100
        anchors.centerIn: parent
        color: "blue"
        enabled: showFps
        visible: enabled

        PropertyAnimation {
            target: rot
            property: "rotation"
            from: 0
            to: 360
            duration: 500
            running: true
            loops: Animation.Infinite
        }
    }

    PropertyAnimation {
        target: rot
        property: "rotation"
        from: 0
        to: 360
        duration: 500
        running: true
        loops: Animation.Infinite
    }
}
