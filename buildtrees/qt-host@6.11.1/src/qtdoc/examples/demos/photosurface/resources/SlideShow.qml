// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: root
    required property url multiFrameSource
    property int pageDuration: 5000

    Image {
        id: imgEven
        anchors {
            horizontalCenter: parent.horizontalCenter
            horizontalCenterOffset: -150
            verticalCenter: parent.verticalCenter
        }
        source: root.multiFrameSource
        sourceSize.width: width
        width: root.width - 400
        opacity: seq.currentFrame % 2 == 0 ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: 500 }
        }
    }

    Image {
        id: imgOdd
        anchors.centerIn: imgEven
        source: root.multiFrameSource
        sourceSize.width: width
        width: imgEven.width
        opacity: 1 - imgEven.opacity
    }

    Rectangle {
        width: Math.min(330, root.width / 5.8)
        height: width / 3.8
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        color: mouse.hovered || tap.pressed ? "white" : "#009b87"
        border.color: mouse.hovered || tap.pressed ?  "#009b87" : "transparent"
        border.width: 2
        Text {
            color: mouse.hovered || tap.pressed ?  "#009b87" : "white"
            anchors.centerIn: parent
            font.pixelSize: parent.width / 10
            text: seq.currentFrame < imgEven.frameCount - 1 ? qsTr("Skip Intro") : qsTr("Ready to go")
        }
        HoverHandler {
            id: mouse
        }
        TapHandler {
            id: tap
            onTapped: root.quit()
        }
    }

    function quit() {
        root.destroy()
    }

    SequentialAnimation {
        id: seq
        loops: Animation.Infinite
        property int currentFrame: -1
        onCurrentFrameChanged: if (currentFrame >= imgEven.frameCount) root.quit()

        ScriptAction {
            script: {
                imgEven.currentFrame = ++seq.currentFrame
            }
        }

        PauseAnimation {
            duration: root.pageDuration
        }

        ScriptAction {
            script: {
                imgOdd.currentFrame = ++seq.currentFrame
            }
        }

        PauseAnimation {
            duration: root.pageDuration
        }
    }

    Component.onCompleted: seq.start()
}
