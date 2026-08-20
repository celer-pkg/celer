// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtQuick.Timeline 1.0

Item {
    Item {
        width: 480
        height: 480

        Timeline {
            id: timeline

            objectName: "timeline"

            startFrame: 0
            endFrame: 100
            currentFrame: 50

            enabled: true

            animations: [
                TimelineAnimation {
                    objectName: "animation"
                    id: animation
                    duration: 200
                    loops: 1
                    from: 0
                    to: 100
                    running: false
                }

            ]

            KeyframeGroup {
                objectName: "group01"
                target: rectangle
                property: "x"

                Keyframe {
                    frame: 0
                    value: 0
                }

                Keyframe {
                    objectName: "keyframe"
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
                }

                Keyframe {
                    objectName: "easingBounce"
                    frame: 100
                    value: 200
                    easing.type: Easing.InBounce
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

            objectName: "rectangle"

            width: 20
            height: 20
            color: "red"
        }
    }
}
