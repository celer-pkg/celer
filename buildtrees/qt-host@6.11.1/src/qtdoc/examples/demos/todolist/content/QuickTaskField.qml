// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQml

QuickTaskFieldForm {
    property date fullDate: new Date()
    property string currentDate: fullDate.toLocaleDateString(Qt.locale(),"dd MMMM yyyy")

    signal addNewQuickTask(str: string, dateStr: string)

    function addTask() {
        addNewQuickTask(taskTitle, currentDate)
        taskTitle = ""
    }

    enterButton.onTapped: addTask()
    newQuickTask.onAccepted: addTask()
}
