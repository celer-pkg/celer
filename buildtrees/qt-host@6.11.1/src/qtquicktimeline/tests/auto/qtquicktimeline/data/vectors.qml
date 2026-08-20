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
        currentFrame: 0
        KeyframeGroup {
            objectName: "group01"
            target: rotation
            property: "axis.x"
            Keyframe {
                frame: 0
                value: 0
            }

            Keyframe {
                frame: 50
                value: 1
            }
            Keyframe {
                frame: 100
                value: 0
            }
        }

        KeyframeGroup {
            target: rotation
            property: "origin.x"
            Keyframe {
                frame: 0
                value: 0
            }

            Keyframe {
                frame: 50
                value: 10
            }
            Keyframe {
                frame: 100
                value: 20
            }
        }

    }

    Text {
        id: rectangle
        x: 220
        y: 140
        width: 300
        height: 300
        transform: Rotation {
            id: rotation
            objectName: "rotation"
            origin.x: 30
            origin.y: 30
            axis.x: 0
            axis.y: 1
            axis.z: 0
            angle: 18
        }


    }

}
