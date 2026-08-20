// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls

Pane {
    id: root

    property alias todayTasksModel: todayTasksListModel
    property alias thisWeekTasksModel: thisWeekTasksListModel
    property alias laterTasksModel: laterTasksListModel

    property alias todaysTasksList: todayTasks
    property alias thisWeekTasksList: thisWeekTasks
    property alias laterTasksList: laterTasks

    property alias isTodayListInEditMode: todayTasks.isListInEditMode
    property alias isThisWeekListInEditMode: thisWeekTasks.isListInEditMode
    property alias isLaterListInEditMode: laterTasks.isListInEditMode

    property alias editedTodaysTaskIndex: todayTasks.editedTaskIndex
    property alias editedThisWeekTaskIndex: thisWeekTasks.editedTaskIndex
    property alias editedLaterTaskIndex: laterTasks.editedTaskIndex

    property int todayTasksMaxHeight: 180
    property int workTasksMaxHeight: 360
    property int doneTasksCount: 0

    padding: 0

    ListModel {
        id: todayTasksListModel
    }

    ListModel {
        id: thisWeekTasksListModel
    }

    ListModel {
        id: laterTasksListModel
    }

    Column {
        id: column

        anchors.fill: parent
        spacing: 14

        TasksList {
            id: todayTasks

            width: column.width
            maxHeight: 180
            listModel: todayTasksListModel
            headerText: qsTr("Today")
            tasksCount: todayTasksListModel.count
        }

        TasksList {
            id: thisWeekTasks

            width: column.width
            maxHeight: column.height - y - 60
            listModel: thisWeekTasksListModel
            headerText: qsTr("This week")
            tasksCount: thisWeekTasksListModel.count
        }

        TasksList {
            id: laterTasks

            width: column.width
            maxHeight: column.height - y
            listModel: laterTasksListModel
            headerText: qsTr("Later")
            tasksCount: laterTasksListModel.count
        }
    }
}
