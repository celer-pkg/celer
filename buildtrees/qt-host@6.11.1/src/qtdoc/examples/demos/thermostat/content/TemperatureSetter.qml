// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import Thermostat

TemperatureSetterForm {
    function saveSettings() {
        let result = Database.getSchedule(scheduleViewRoot.currentRoomIndex,
                                          scheduleViewRoot.currentDate)
        if (result) {
            Database.updateSchedule(scheduleViewRoot.currentRoomIndex,
                                    scheduleViewRoot.currentDate,
                                    scheduleViewRoot.selectedHours.toString(),
                                    scheduleViewRoot.selectedDays.toString(),
                                    scheduleViewRoot.currentTemp,
                                    scheduleViewRoot.currentMode)
        } else {
            Database.saveSchedule(scheduleViewRoot.currentRoomIndex,
                                  scheduleViewRoot.currentDate,
                                  scheduleViewRoot.selectedHours.toString(),
                                  scheduleViewRoot.selectedDays.toString(),
                                  scheduleViewRoot.currentTemp,
                                  scheduleViewRoot.currentMode)
        }
    }

    function cleanSettings() {
        scheduleViewRoot.selectedHours = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        scheduleViewRoot.selectedDays = [0,0,0,0,0,0,0]
        scheduleViewRoot.currentTemp = 10
        scheduleViewRoot.currentMode = 2
    }

    saveButtonDesktop.onClicked: saveSettings()
    saveButtonMobile.onClicked: saveSettings()
    saveButtonSmall.onClicked: saveSettings()

    cancelButtonDesktop.onClicked: cleanSettings()
    cancelButtonMobile.onClicked: cleanSettings()
    cancelButtonSmall.onClicked: cleanSettings()
}
