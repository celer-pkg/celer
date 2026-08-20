// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import ToDoList

HomePageForm {

    function cleanDoneTasks(tasks : ListModel) : void {
        for (let j = 0; j < tasks.count; j++) {
            if (tasks.get(j).done) {
                tasks.remove(j)
                j--
            }
        }
    }

    function getTargetList(dueDateText : string) : ListModel {
        var currentDate = new Date()
        var format = Qt.locale().dateFormat(Locale.LongFormat)
        var currentDateStr = currentDate.toLocaleDateString(Qt.locale(), format)
        var destList = undefined

        if (currentDateStr === dueDateText) {
            destList = todayTasksModel
        } else if (taskList.checkThisWeekDate(dueDateText)) {
            destList = thisWeekTasksModel
        } else {
            destList = laterTasksModel
        }

        return destList
    }

    function addTask(tasksList : ListModel, titleText : string, dueDateText : string,
                     dueTimeText : string, notes : string) : void {
        if (taskList.getTasksCount() < AppSettings.maxTasksCount) {
            const task_id = Database.addTask(titleText, dueDateText, dueTimeText, notes, 0, 0)
            tasksList.insert(0, { "task_id": task_id,
                                  "name": titleText,
                                  "date": dueDateText,
                                  "time" : dueTimeText,
                                  "notes": notes,
                                  "highlight": 0,
                                  "done": 0 })
        } else {
            infoLabel.visible = true
        }
    }

    function updateTask(sourceList : ListModel, destList : ListModel, index : string,
                       title : string, dueDate : string, dueTime : string, notes : string) : void {
        Database.updateTask(sourceList.get(index).task_id, title, dueDate, dueTime, notes)

        sourceList.set(index, {"name" : title,
                               "date" : dueDate,
                               "time" : dueTime,
                               "notes" : notes})

        if (sourceList != destList) {
            let taskObj = sourceList.get(index)
            destList.insert(0, taskObj)
            sourceList.remove(index, 1)
        }
    }

    function sendHttpRequest() : void {
        var http = new XMLHttpRequest()
        var url = "https://www.boredapi.com/api/activity";
        http.open("GET", url, true);

        http.setRequestHeader("Content-type", "application/json");
        http.setRequestHeader("Connection", "close");

        http.onreadystatechange = function() {
            if (http.readyState == 4) {
                if (http.status == 200) {
                    let object = JSON.parse(http.responseText.toString());
                    let currentDate = new Date()
                    let format = Qt.locale().dateFormat(Locale.LongFormat)
                    addTask(todayTasksModel, object.activity,
                            currentDate.toLocaleDateString(Qt.locale(), format), "","")
                } else {
                    console.log("Failed to fetch a random TODO item; the API provider might be down")
                }
            }
        }
        http.send();
    }

    addButton.onTapped: {
        isEditMode = false
        StackView.view.push(newTask)
    }

    settingsButton.onClicked: {
        StackView.view.push(settings)
    }

    editModeButton.onTapped: {
        isLaterListInEditMode = false
        isTodayListInEditMode = false
        isThisWeekListInEditMode = false
    }

    randomActivityButton.onTapped: sendHttpRequest()

    taskList.onEditTask: (index, listModel) => {
        var currentTask = listModel.get(index)
        var date = Date.fromLocaleDateString(Qt.locale(), currentTask.date, Locale.LongFormat)

        isEditMode = true
        currentTaskIndex = index
        titleText = currentTask.name
        dueDateText = date.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
        newTask.calendar.selectedDate = date
        dueTimeText = currentTask.time
        notes = currentTask.notes
        sourceModel = listModel
        StackView.view.push(newTask)
    }

    quickTask.onAddNewQuickTask: (taskStr, dateStr) => {
            addTask(todayTasksModel, taskStr, dateStr, "", "")
    }

    newTask.onNewTaskCreated: (titleText, dueDateText, dueTimeText, notes) => {
            var destList = getTargetList(dueDateText)
            addTask(destList, titleText, dueDateText, dueTimeText, notes)
    }

    newTask.onTaskUpdated: (index, titleText, dueDateText, dueTimeText, notes) => {
        var destList = getTargetList(dueDateText)
        updateTask(sourceModel, destList, index, titleText, dueDateText, dueTimeText, notes)
    }

    clearTasks.onTapped: {
        cleanDoneTasks(laterTasksModel)
        cleanDoneTasks(thisWeekTasksModel)
        cleanDoneTasks(todayTasksModel)
        Database.deleteDoneTasks()
    }
}
