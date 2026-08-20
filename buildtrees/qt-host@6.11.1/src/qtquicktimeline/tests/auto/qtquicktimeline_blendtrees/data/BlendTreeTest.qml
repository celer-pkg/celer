// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Timeline
import QtQuick.Timeline.BlendTrees

Item {
    Item {
        width: 480
        height: 480

        TimelineAnimation {
            objectName: "animation1"
            id: animation1
            duration: 20000
            loops: -1
            from: 0
            to: 100
        }
        TimelineAnimation {
            objectName: "animation2"
            id: animation2
            duration: 20000
            loops: -1
            from: 100
            to: 200
        }

        Timeline {
            id: timeline
            objectName: "timeline"

            enabled: true

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

                Keyframe {
                    frame: 150
                    value: 100
                }

                Keyframe {
                    frame: 200
                    value: 0
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

                Keyframe {
                    frame: 150
                    value: 300
                }

                Keyframe {
                    frame: 200
                    value: 400
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

                Keyframe {
                    frame: 150
                    value: "cyan"
                }

                Keyframe {
                    frame: 200
                    value: "magenta"
                }
            }
        }

        TimelineAnimationNode {
            id: animation1Node
            objectName: "animation1Node"
            timeline: timeline
            animation: animation1
        }

        TimelineAnimationNode {
            id: animation2Node
            objectName: "animation2Node"
            timeline: timeline
            animation: animation2
        }

        BlendAnimationNode {
            id: animationBlendNode
            objectName: "blendAnimation"
            source1: animation1Node
            source2: animation2Node
            weight: 0.5
            outputEnabled: true
        }

        Rectangle {
            id: rectangle

            objectName: "rectangle"

            width: 20
            height: 20
            color: "red"
        }

        AnimationController {
            id: animation1Controller
            objectName: "animation1Controller"
            animation: animation1
        }
        AnimationController {
            id: animation2Controller
            objectName: "animation2Controller"
            animation: animation2
        }
    }
}
