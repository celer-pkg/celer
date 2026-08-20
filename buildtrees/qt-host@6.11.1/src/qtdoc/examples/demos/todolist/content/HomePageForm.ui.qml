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
import QtQuick.Layouts
import ToDoList
import CustomControls

Page {
    id: root

    property var builtInStyles

    property alias quickTask: quickTask
    property alias addButton: addButton
    property alias settingsButton: settingsButton
    property alias settings: settings
    property alias newTask: newTask
    property alias taskList: tasksList
    property alias randomActivityButton: randomTaskButton
    property alias infoLabel: infoLabel
    property alias isEditMode: newTask.isEditMode
    property alias sourceModel: newTask.sourceModel
    property alias currentTaskIndex: newTask.currentTaskIndex
    property alias titleText: newTask.titleText
    property alias dueDateText: newTask.dueDateText
    property alias dueTimeText: newTask.dueTimeText
    property alias notes: newTask.notes
    property alias laterTasksModel: tasksList.laterTasksModel
    property alias todayTasksModel: tasksList.todayTasksModel
    property alias thisWeekTasksModel: tasksList.thisWeekTasksModel
    property alias isLaterListInEditMode: tasksList.isLaterListInEditMode
    property alias isTodayListInEditMode: tasksList.isTodayListInEditMode
    property alias isThisWeekListInEditMode: tasksList.isThisWeekListInEditMode
    property alias clearTasks: clearDoneLabel.tapHandler
    property alias editModeButton: turnOffEditModeLabel.tapHandler

    padding: 0

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 20

            Image {
                Layout.leftMargin: 6

                source: "images/Qt_Icon.svg"
            }

            Label {
                Layout.fillWidth: true

                text: qsTr("Tasks")
                color: Constants.secondaryColor
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                font.pixelSize: AppSettings.fontSize + 4
                font.bold: true
            }

            ToolButton {
                id: settingsButton

                icon.source: "images/Settings_Icon.svg"

                palette.button: Constants.isDarkModeActive ? "#30D158" : "#34C759"
                palette.highlight: Constants.isDarkModeActive ? "#30DB5B" : "#248A3D"
            }
        }
    }

    QuickTaskField {
        id: quickTask

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Row {
        id: labels

        anchors.top: quickTask.bottom
        anchors.right: parent.right
        anchors.rightMargin: 32

        height: clearDoneLabel.height
        layoutDirection: Qt.RightToLeft
        spacing: 20

        CustomLabel {
            id: clearDoneLabel

            text: qsTr("Clear Done: %1").arg(tasksList.doneTasksCount)
            visible: tasksList.doneTasksCount
        }

        CustomLabel {
            id: turnOffEditModeLabel

            text: qsTr("Ok")
            visible: tasksList.isTodayListInEditMode
                     || tasksList.isThisWeekListInEditMode
                     || tasksList.isLaterListInEditMode
        }
    }

    TasksListsView {
        id: tasksList

        anchors.top: labels.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 90
        anchors.topMargin: 10
        anchors.rightMargin: 16
        anchors.leftMargin: 16
    }

    Row {
        id: buttons

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 20
        anchors.bottomMargin: 20

        spacing: 5

        CustomButton {
            id: randomTaskButton

            iconSource: "images/Random_Task_Icon.svg"
        }

        CustomButton {
            id: addButton

            iconSource: "images/Add_Icon.svg"
        }
    }

    NewTask {
        id: newTask

        visible: false
    }

    SettingsView {
        id: settings

        visible: false
        builtInStyles: root.builtInStyles
    }

    ToolTip {
        id: infoLabel

        anchors.centerIn: parent

        timeout: 4000
        text: qsTr("Max number of tasks achieved!")
    }
}
