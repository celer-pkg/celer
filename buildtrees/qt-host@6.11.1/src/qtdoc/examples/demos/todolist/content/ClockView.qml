// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

ClockViewForm {
    cancel.onTapped: visible = false

    timeInput.minTextField.onPressed: isHourClock = false
    timeInput.hourTextField.onPressed: isHourClock = true

    timeInput.hourTextField.onAccepted: {
        var hour = parseInt(hourText)
        angle = hour < 6 ? hour * 30 : hour * 30 - 360
        isHourClock = false
    }

    timeInput.minTextField.onAccepted: {
        var minutes = parseInt(minText)
        angle = minutes < 30 ? minutes * 6 : minutes * 6 - 360
    }

    clock.onHourChanged: hourText = clockHour < 10 ? "0" + clockHour : clockHour
    clock.onMinChanged: minText = clockMin < 10 ? "0" + clockMin : clockMin
}
