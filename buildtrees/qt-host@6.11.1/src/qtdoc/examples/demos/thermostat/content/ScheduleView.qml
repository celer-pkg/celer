// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import Thermostat

ScheduleViewForm {
    id: root
    function getCurrentDate() {
        let currentDate = new Date()
        return currentDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
    }

    function setDefaultValues() {
        currentMode = 2
        currentTemp = 10
        selectedDays = [0,0,0,0,0,0,0]
        selectedHours = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    }

    function setValues(mode: int, temp: int, hours: string, days: string) {
        currentMode = mode
        currentTemp = temp
        selectedDays = JSON.parse("[" + days + "]")
        selectedHours = JSON.parse("[" + hours + "]")
    }

    currentDate: getCurrentDate()

    onCurrentDateChanged: {
        let result = Database.getSchedule(currentRoomIndex, currentDate)
        if (result) {
            setValues(result.mode, result.temp, result.hours, result.days)
        } else {
            setDefaultValues()
        }
    }

    onCurrentRoomIndexChanged: {
        let result = Database.getSchedule(currentRoomIndex, currentDate)
        if (result) {
            setValues(result.mode, result.temp, result.hours, result.days)
        } else {
            setDefaultValues()
        }
    }

    scheduleViewRootForScrollView: root
    scheduleViewRootForSwipView: root
}
