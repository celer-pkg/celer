// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Timeline

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

            enabled: false

            KeyframeGroup {
                objectName: "group01"
                target: rectangle
                property: "x"

                Keyframe {
                    frame: 0
                    value: 140
                }

                Keyframe {
                    frame: 100
                    value: rectangle.offset
                }
            }

            KeyframeGroup {
                target: rectangle
                property: "anchors.topMargin"

                Keyframe {
                    frame: 0
                    value: rectangle.offset
                }

                Keyframe {
                    frame: 100
                    value: 140
                }
            }
        }

        Rectangle {
            id: rectangle
            objectName: "rectangle"
            property int offset: 0

            x: offset
            width: 20
            height: 20
            color: "red"
            anchors.top: parent.top
            anchors.topMargin: offset
        }
    }
}
