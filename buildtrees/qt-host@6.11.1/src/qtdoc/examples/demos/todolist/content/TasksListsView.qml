// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import ToDoList

TasksListsViewForm {

    signal editTask(int index, ListModel listModel)

    doneTasksCount: getAllDoneTasksCount()

    function getTasksCount() : int {
        return todayTasksModel.count +
               laterTasksModel.count +
               thisWeekTasksModel.count
    }

    function getAllDoneTasksCount() : int {
        var count = getDoneTasksCount(todayTasksModel)
        count += getDoneTasksCount(laterTasksModel)
        count += getDoneTasksCount(thisWeekTasksModel)
        return count
    }

    function getDoneTasksCount(listModel) : int {
        var count = 0
        for (let i = 0; i < listModel.count; i++) {
            if (listModel.get(i).done) {
                count++
            }
        }
        return count
    }

    function checkThisWeekDate(taskDate : string) : bool {
        var dateTmp = new Date()
        var format = Qt.locale().dateFormat(Locale.LongFormat)
        for (let i = 1; i <= 7; i++) {
            dateTmp.setDate(dateTmp.getDate() + 1)
            let dateStr = dateTmp.toLocaleDateString(Qt.locale(),format)
            if (taskDate === dateStr) {
                return true
            }
        }
        return false
    }

    function createTasksLists() : void {
        var tasks = Database.getTasks()
        var currentDate = new Date()
        var format = Qt.locale().dateFormat(Locale.LongFormat)
        var dateStr = currentDate.toLocaleDateString(Qt.locale(),format)
        tasks.forEach( function(task){
            if (task.date === dateStr) {
                todayTasksModel.append(task)
            } else if (checkThisWeekDate(task.date)) {
                thisWeekTasksModel.append(task)
            } else {
                laterTasksModel.append(task)
            }
        })
    }

    todaysTasksList.onEditedTaskIndexChanged: {
        if (editedTodaysTaskIndex !== -1) {
            editTask(editedTodaysTaskIndex, todayTasksModel)
            editedTodaysTaskIndex = -1
        }
    }

    laterTasksList.onEditedTaskIndexChanged: {
        if (editedLaterTaskIndex !== -1) {
            editTask(editedLaterTaskIndex, laterTasksModel)
            editedLaterTaskIndex = -1
        }
    }

    thisWeekTasksList.onEditedTaskIndexChanged: {
        if (editedThisWeekTaskIndex !== -1) {
            editTask(editedThisWeekTaskIndex, thisWeekTasksModel)
            editedThisWeekTaskIndex = -1
        }
    }

    Component.onCompleted: createTasksLists()
}
