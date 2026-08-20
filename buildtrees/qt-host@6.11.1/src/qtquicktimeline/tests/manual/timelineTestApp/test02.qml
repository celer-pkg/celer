// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtQuick.Timeline 1.0

Item {

    Rectangle {
        id: leftGauge
        x: 20
        y: 140
        width: 200
        height: 200
        color: "#969696"
        radius: width / 2

        Rectangle {
            x: 5
            y: 5
            width: 190
            height: 190
            color: "#0d0d0d"
            radius: width /2
        }
    }

    Rectangle {
        id: rightGauge
        x: 420
        y: 140
        width: 200
        height: 200
        color: "#969696"
        radius: width / 2
        Rectangle {
            x: 5
            y: 5
            width: 190
            height: 190
            color: "#0d0d0d"
            radius: width /2
        }
    }

    Rectangle {
        id: bottomPane
        x: 81
        y: 424
        width: 478
        height: 48
        color: "#242424"
    }

    Timeline {
        id: timeline
        enabled: true

        startFrame: 0
        endFrame: 1000

        animations: [

            TimelineAnimation {
                running: true
                duration: 1000
                from: 0
                to: 1000
            }
        ]

        KeyframeGroup {
            target: leftGauge
            property: "x"
            Keyframe {
                frame: 0
                value: -200
            }
            Keyframe {
                frame: 500
                value: 0
                easing.type: Easing.InQuad
            }

            Keyframe {
                frame: 1000
                value: 20
                easing.type: Easing.OutQuad
            }
        }
        KeyframeGroup {
            target: leftGauge
            property: "y"
            Keyframe {
                frame: 0
                value: 280
            }

            Keyframe {
                frame: 500
                value: 226
                easing.type: Easing.InQuad
            }
            Keyframe {
                frame: 1000
                value: 140
                easing.type: Easing.OutQuad
            }
        }
        KeyframeGroup {
            target: leftGauge
            property: "opacity"
            Keyframe {
                frame: 0
                value: 0
            }
            Keyframe {
                frame: 500
                value: 0.2
                easing.type: Easing.InQuad
            }
            Keyframe {
                frame: 1000
                value: 1
                easing.type: Easing.OutQuad
            }
        }

        KeyframeGroup {
            target: rightGauge
            property: "x"
            Keyframe {
                frame: 0
                value: 639
            }
            Keyframe {
                frame: 500
                value: 440
                easing.type: Easing.InQuad
            }
            Keyframe {
                frame: 1000
                value: 420
                easing.type: Easing.OutQuad
            }
        }
        KeyframeGroup {
            target: rightGauge
            property: "y"
            Keyframe {
                frame: 0
                value: 280
            }
            Keyframe {
                frame: 500
                value: 226
                easing.type: Easing.InQuad
            }
            Keyframe {
                frame: 1000
                value: 140
                easing.type: Easing.OutQuad
            }
        }
        KeyframeGroup {
            target: rightGauge
            property: "opacity"
            Keyframe {
                frame: 0
                value: 0.0
            }
            Keyframe {
                frame: 500
                value: 0.2
                easing.type: Easing.InQuad
            }
            Keyframe {
                frame: 1000
                value: 1.0
                easing.type: Easing.OutQuad
            }
        }

        KeyframeGroup {
            target: bottomPane
            property: "y"

            Keyframe {
                frame: 0
                value: 502
            }
            Keyframe {
                frame: 500
                value: 432
                easing.type: Easing.InQuad
            }

            Keyframe {
                frame: 1000
                value: 424
                easing.type: Easing.OutQuad
            }
        }
        KeyframeGroup {
            target: bottomPane
            property: "opacity"
            Keyframe {
                frame: 0
                value: 0
            }
            Keyframe {
                frame: 500
                value: 0.7
                easing.type: Easing.InQuad
            }
            Keyframe {
                frame: 1000
                value: 1
                easing.type: Easing.OutQuad
            }
        }
    }

}
