// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQml

NewTaskForm {

    property string localeDateFormat: Qt.locale().dateFormat(Locale.ShortFormat)
    property string localeLongDateFormat: Qt.locale().dateFormat(Locale.LongFormat)
    property string localeTimeFormat: Qt.locale().timeFormat(Locale.ShortFormat)

    signal newTaskCreated(title : string, dueDate: string, dueTime: string, notes: string)
    signal taskUpdated(index: int, title : string, dueDate: string, dueTime: string, notes: string)

    function closeView() : void {
        titleText = ""
        dueTimeText = ""
        dueDateText = ""
        notes = ""
        isHourClock = true
        StackView.view.pop()
    }

    function addTask() : void {
        if (isEditMode) {
            updateTask(currentTaskIndex)
        } else {
            createTask()
        }
        isHourClock = true
    }

    function createTask() : void {
        if (titleText) {
            let date = Date.fromLocaleDateString(Qt.locale(), dueDateText, localeDateFormat)
            let dateStr = date.toLocaleDateString(Qt.locale(), localeLongDateFormat)

            newTaskCreated(titleText, dateStr, dueTimeText, notes)
            closeView()
        }
    }

    function updateTask(index : int) : void {
        if (titleText) {
            let date = Date.fromLocaleDateString(Qt.locale(), dueDateText, localeDateFormat)
            if (isNaN(date)) {
               date = Date.fromLocaleDateString(Qt.locale(), dueDateText, localeLongDateFormat)
            }

            let dateStr = date.toLocaleDateString(Qt.locale(), localeLongDateFormat)

            taskUpdated(index, titleText, dateStr, dueTimeText, notes)
            closeView()
        }
    }

    datePlaceholderText: localeDateFormat
    timePlaceholderText: localeTimeFormat

    calendarButton.onTapped: calendar.visible = true
    clockButton.onTapped: clock.visible = true
    backButton.onClicked: closeView()

    calendar.ok.onTapped: {
        calendar.visible = false
        dueDateText = selectedDate.toLocaleDateString(Qt.locale(), localeDateFormat)
    }

    clock.accept.onTapped: {
        clock.visible = false
        var timeText = hourText + ":" + minText + " " + amPmText
        var time = Date.fromLocaleTimeString(Qt.locale(), timeText, "h:mm AP")
        dueTimeText = time.toLocaleTimeString(Qt.locale(), localeTimeFormat)
    }

    acceptButton.onClicked: addTask()
    addButton.onClicked: addTask()
}
