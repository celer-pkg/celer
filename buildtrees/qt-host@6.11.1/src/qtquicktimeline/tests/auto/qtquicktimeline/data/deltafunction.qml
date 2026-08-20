// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtQuick.Timeline 1.0

Item {
    width: 480
    height: 480

    Timeline {
        id: timeline

        objectName: "timeline"

        startFrame: 0
        endFrame: 100
        currentFrame: 0

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
            target: text
            property: "text"

            Keyframe {
                frame: 0
                value: "frame0"
            }

            Keyframe {
                frame: 50
                value: "frame50"
            }

            Keyframe {
                frame: 100
                value: "frame100"
            }
        }
    }

    Text {
        id: text
        objectName: "text"
        text: "no timeline"
    }
}
