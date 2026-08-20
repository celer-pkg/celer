// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtQuick.Timeline 1.0

Item {
    width: 640
    height: 480

    id: root

    state: "onPage02"

    Rectangle {
        id: rectangle
        x: 0
        y: 0
        width: 64
        height: 64
        color: "#747474"

        MouseArea {
            anchors.fill: parent
            id: leftArea
        }
    }

    Rectangle {
        id: rectangle1
        x: 576
        y: 0
        width: 64
        height: 64
        color: "#747474"

        MouseArea {
            anchors.fill: parent
            id: rightArea
        }
    }

    Item {
        id: item1
        x: -640
        y: 123
        width: 1920
        height: 480

        Rectangle {
            id: page01
            x: 0
            y: 0
            width: 640
            height: 359
            color: "#ffffff"

            Text {
                x: 0
                y: 0
                text: qsTr("Page 01")
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 38
            }
        }

        Rectangle {
            id: page02
            x: 640
            y: 0
            width: 640
            height: 359
            color: "#ffffff"

            Text {
                x: 0
                y: 0
                text: qsTr("Page 02")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 38
            }
        }

        Rectangle {
            id: page03
            x: 1280
            y: 0
            width: 640
            height: 359
            color: "#ffffff"

            Text {
                x: 0
                y: 0
                text: qsTr("Page 03")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 38
            }
        }
    }

    Timeline {
        id: timeline
        endFrame: 1000
        startFrame: 0
        enabled: true

        animations: [
            TimelineAnimation {
                id: animationToPage02FromLeft
                from: 0
                to: 500
                running: false
                onFinished: root.state = "onPage02"
            },
            TimelineAnimation {
                id: animationToPage03FromLeft
                from: 500
                to: 1000
                running: false
                onFinished: root.state = "onPage03"
            },
            TimelineAnimation {
                id: animationToPage02FromRight
                from: 1000
                to: 500
                running: false
                onFinished: root.state = "onPage02"
            },
            TimelineAnimation {
                id: animationToPage01FromRight
                from: 500
                to: 0
                running: false
                onFinished: root.state = "onPage01"
            }
        ]

        KeyframeGroup {
            target: item1
            property: "x"

            Keyframe {
                frame: 0
                value: 0
            }

            Keyframe {
                frame: 500
                value: -640
            }

            Keyframe {
                frame: 1000
                value: -1280
            }
        }
    }

    Connections {
        target: rightArea
        enabled: root.state == "onPage01"
        onClicked: root.state = "toPage02FromLeft"
    }

    Connections {
        target: rightArea
        enabled: root.state == "onPage02"
        onClicked: root.state = "toPage03FromLeft"
    }

    Connections {
        target: leftArea
        enabled: root.state == "onPage02"
        onClicked: root.state = "toPage01FromRight"
    }

    Connections {
        target: leftArea
        enabled: root.state == "onPage03"
        onClicked: root.state = "toPage02FromRight"
    }

    states: [
        State {
            name: "onPage01"
            PropertyChanges {
                target: timeline
                currentFrame: 0
            }
        },
        State {
            name: "onPage02"
            PropertyChanges {
                target: timeline
                currentFrame: 500
            }
        },
        State {
            name: "onPage03"
            PropertyChanges {
                target: timeline
                currentFrame: 1000
            }
        },
        State {
            name: "toPage01FromRight"
            PropertyChanges {
                target: animationToPage01FromRight
                running: true
            }
        },
        State {
            name: "toPage02FromLeft"
            PropertyChanges {
                target: animationToPage02FromLeft
                running: true
            }
        },
        State {
            name: "toPage03FromLeft"
            PropertyChanges {
                target: animationToPage03FromLeft
                running: true
            }
        },
        State {
            name: "toPage02FromRight"
            PropertyChanges {
                target: animationToPage02FromRight
                running: true
            }
        }
    ]
}

/*##^## Designer {
    D{i:62;anchors_width:100;anchors_height:100}D{i:47;currentFrame__AT__NodeInstance:1}
}
 ##^##*/
