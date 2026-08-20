// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtQuick.Timeline 1.0

Item {
    Rectangle {
        width: 40
        height: 40
        color: "blue"
        MouseArea {
            anchors.fill: parent
            onClicked: timeline.enabled = !timeline.enabled
        }
    }

    Rectangle {
        x: 80
        width: 40
        height: 40
        color: "blue"
        MouseArea {
            anchors.fill: parent
            onClicked: animation.restart()
        }
    }

    Item {
        width: 480
        height: 480

        Timeline {
            id: timeline

            startFrame: 0
            endFrame: 100
            currentFrame: 50

            enabled: true

            animations: [
                TimelineAnimation {
                    id: animation
                    duration: 2000
                    from: 0
                    to: 100
                    running: false
                }

            ]

            KeyframeGroup {
                target: rectangle
                property: "x"

                Keyframe {
                    frame: 0
                    value: 0
                }

                Keyframe {
                    frame: 50
                    value: 100
                }

                Keyframe {
                    frame: 100
                    value: 200
                }
            }

            KeyframeGroup {
                target: rectangle
                property: "y"

                Keyframe {
                    frame: 0
                    value: 0
                }

                Keyframe {
                    frame: 50
                    value: 100
                    easing.type: Easing.InBounce
                }

                Keyframe {
                    frame: 100
                    value: 200
                }
            }

            KeyframeGroup {
                target: rectangle
                property: "color"

                Keyframe {
                    frame: 0
                    value: "red"
                }

                Keyframe {
                    frame: 50
                    value: "blue"
                }

                Keyframe {
                    frame: 100
                    value: "yellow"
                }
            }
        }

        Rectangle {
            id: rectangle
            width: 20
            height: 20
            color: "red"
        }
    }
}
