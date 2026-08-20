// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtQuick.Timeline 1.0

Item {
    id: item1
    state: "pingpong"

    Timeline {
        id: timeline
        enabled: true
        startFrame: 0
        endFrame: 1000
        animations: [
            TimelineAnimation {
                id: pingPongAnimation
                to: 200
                loops: 2
                from: 0
                duration: 2000
                running: false
                pingPong: true
                onFinished: item1.state = "firsthalf"
            },

            TimelineAnimation {
                id: animation01
                to: 200
                loops: 1
                from: 0
                duration: 1000
                running: false
                onFinished: item1.state = "secondhalf (ping pong)"
            },
            TimelineAnimation {
                id: animation02
                to: 400
                loops: 1
                from: 200
                duration: 1000
                running: false
                pingPong: true
                onFinished: item1.state = "last"
            },
            TimelineAnimation {
                id: animation03
                to: 0
                loops: 1
                from: 200
                duration: 500
                running: false
                onFinished: item1.state = "pingpong"
            }
        ]

        KeyframeGroup {
            target: rectangle
            property: "width"
            Keyframe {
                frame: 1
                value: 50
            }
        }

        KeyframeGroup {
            target: rectangle
            property: "height"
            Keyframe {
                frame: 1
                value: 50
            }
        }

        KeyframeGroup {
            target: rectangle
            property: "x"
            Keyframe {
                frame: 100
                value: 100
            }

            Keyframe {
                frame: 200
                value: 200
            }

            Keyframe {
                frame: 300
                value: 100
            }

            Keyframe {
                frame: 400
                value: 0
            }
        }

        KeyframeGroup {
            target: rectangle
            property: "y"
            Keyframe {
                frame: 100
                value: 400
            }

            Keyframe {
                frame: 200
                value: 430
            }

            Keyframe {
                frame: 300
                value: 335
            }

            Keyframe {
                frame: 400
                value: 430
            }
        }

        KeyframeGroup {
            target: rectangle
            property: "color"

            Keyframe {
                frame: 400
                value: "#f61b1b"
            }
        }
    }

    Rectangle {
        id: rectangle
        x: 0
        y: 430
        width: 50
        height: 50
        color: "#f61b1b"
        MouseArea {
            anchors.topMargin: 109
            anchors.fill: parent
            onClicked: {
                print("clicked")
                numberAnimation.start()
            }
        }
    }

    Rectangle {
        id: rectangle1
        x: 0
        y: 0
        width: 85
        height: 85
        color: "#22f4dd"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                print("clicked")
                numberAnimation.stop()
            }
        }
    }

    Text {
        id: text1
        x: 376
        y: 18
        text: item1.state
        font.pixelSize: 12
    }
    states: [
        State {
            name: "pingpong"

            PropertyChanges {
                target: pingPongAnimation
                running: true
            }
        },
        State {
            name: "firsthalf"
            PropertyChanges {
                target: animation01
                running: true
            }
        },
        State {
            name: "secondhalf (ping pong)"
            PropertyChanges {
                target: animation02
                running: true
            }
        },
        State {
            name: "last"
            PropertyChanges {
                target: animation03
                running: true
            }
        }
    ]

}
