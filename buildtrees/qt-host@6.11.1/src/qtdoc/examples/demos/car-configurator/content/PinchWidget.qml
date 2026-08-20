// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

PinchArea {
    id: pinchArea
    required property PerspectiveCamera camera
    anchors.fill: parent

    onPinchUpdated: pinchEvent => {
        const threshold = 0.008
        if (pinchEvent.previousScale - pinchEvent.scale > threshold
         || pinchEvent.previousScale - pinchEvent.scale < -1 * threshold) {
            let velocity = (pinchEvent.previousScale - pinchEvent.scale) > 0 ? 1.25 : -1.25
            camera.fieldOfView += velocity * (camera.fieldOfView + velocity > 0.0)
                                                 * (camera.fieldOfView + velocity < 120.0)
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        preventStealing: false
        cursorShape: Qt.SizeAllCursor
        drag.axis: Drag.YAxis

        Connections {
            target: mouseArea

            function onWheel(wheel) {
                pinchArea.camera.fieldOfView += wheel.angleDelta.y * 0.04
                * (pinchArea.camera.fieldOfView + wheel.angleDelta.y * 0.04 > 0.0)
                * (pinchArea.camera.fieldOfView + wheel.angleDelta.y * 0.04 < 120.0)
            }
        }
    }
}
