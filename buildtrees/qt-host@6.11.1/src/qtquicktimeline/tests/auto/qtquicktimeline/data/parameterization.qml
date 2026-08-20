// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtQuick.Timeline 1.0

Item {
    id: item1

    Timeline {
        objectName: "timeline"
        id: timeline
        enabled: true

        startFrame: 0
        endFrame: 200
        currentFrame: input.text
        KeyframeGroup {
            objectName: "group01"
            target: needle
            property: "rotation"
            Keyframe {
                frame: 0
                value: 0
            }

            Keyframe {
                frame: 100
                value: 90
            }
            Keyframe {
                frame: 200
                value: 180
            }
        }

        KeyframeGroup {
            target: needle
            property: "color"
            Keyframe {
                frame: 0
                value: "blue"
            }

            Keyframe {
                frame: 100
                value: "green"
            }
            Keyframe {
                frame: 200
                value: "red"
            }
        }

    }

    Rectangle {
        id: rectangle
        x: 220
        y: 140
        width: 300
        height: 300
        color: "#000000"
        radius: 150
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Rectangle {
            objectName: "needle"
            id: needle
            x: 0
            y: 148
            width: 150
            height: 4
            color: "#c41616"
            transformOrigin: Item.Right
        }
    }

    TextInput {
        objectName: "textInput"
        id: input
        x: 207
        y: 392
        width: 227
        height: 65
        text: "10"
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 14
    }
}
