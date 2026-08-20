// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import ToDoList

ClockForm {
    onIsHourClockChanged: angle = isHourClock ? hourClockRotation : minutesClockRotation

    function rotateClockHand(mousePosition: vector2d, centerPoint: vector2d) {
        var vec = centerPoint.minus(mousePosition)
        var rad = Math.atan2(vec.x, vec.y)
        var degree = -(rad) * (180/Math.PI)
        angle = Math.floor(degree / 30) * 30
    }

    clockFace.source: isHourClock ? Constants.clockIconSource : Constants.minClockIconSource

    clockHand.onRotationChanged: {
        if (isHourClock) {
            hour = angle > 0 ? angle / 30 : (angle + 360) / 30
            hourClockRotation = angle
        } else {
            min = angle > 0 ? angle / 30 * 5 : ((angle + 360) / 30 * 5) % 60
            minutesClockRotation = angle
        }
    }

    mouseArea.onPressed: rotateClockHand(Qt.vector2d(mouseX, mouseY), centerPoint)
    mouseArea.onPositionChanged: rotateClockHand(Qt.vector2d(mouseX, mouseY), centerPoint)
}
