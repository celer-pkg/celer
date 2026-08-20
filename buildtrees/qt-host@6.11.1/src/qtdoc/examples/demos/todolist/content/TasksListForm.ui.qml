// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ToDoList
import CustomControls

Column {
    id: root

    property alias taskList: tasksList
    property alias hideTasks: hideTasks
    property alias listModel: tasksList.model
    property alias headerText: tasksLabel.text
    property alias tasksCount: tasksCount.text
    property alias dialogPopup: dialog
    property alias hideAnim: hideTasksList
    property alias showAnim: showTasksList

    property int maxHeight: 400
    property bool isListInEditMode: false
    property int editedTaskIndex: -1
    property var taskData

    spacing: 24

    RowLayout {
        id: row

        width: root.width
        height: 20
        spacing: 8

        Label {
            id: tasksLabel

            text: qsTr("Today")
            color: Constants.secondaryColor
            font.bold: true
            font.pixelSize: AppSettings.fontSize - 2
        }

        Label {
            id: tasksCount

            text: tasksList.model.count
            leftPadding: 6
            rightPadding: 6
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: AppSettings.fontSize - 2
            color: Constants.secondaryColor

            background: Rectangle {
                color: Constants.secondaryColor
                radius: 6
                opacity: 0.2
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Image {
                source: tasksList.visible ? "images/HideTasks_Icon.svg" : "images/ShowTasks_Icon.svg"
                anchors.right: parent.right

                TapHandler {
                    id: hideTasks
                }
            }
        }
    }

    ListView {
        id: tasksList

        width: root.width
        height: contentHeight > root.maxHeight ? root.maxHeight : contentHeight
        spacing: 10
        clip: true

        delegate: TasksListDelegate {
            model: tasksList.model
            implicitWidth: tasksList.width
            isEditMode: root.isListInEditMode

            tasksListArea.onLongPressed: root.isListInEditMode = true
            tasksListArea.onTapped: root.editedTaskIndex = index

            Connections {
                function onTaskToRemove(taskData) {
                    root.taskData = taskData
                }
            }
        }

        remove: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 500
            }
        }

        add: Transition {
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation {
                        property: "opacity"
                        from: 0.0
                        duration: 500
                    }
                    NumberAnimation {
                        property: "scale"
                        from: 0.0
                        to: 1.1
                        duration: 500
                    }
                }
                NumberAnimation {
                    property: "scale"
                    from: 1.1
                    to: 1.0
                    duration: 500
                }
            }
        }

        displaced: Transition {
            NumberAnimation {
                properties: "y"
                duration: 600
                easing.type: Easing.OutBounce
            }
            NumberAnimation {
                properties: "scale"
                to: 1.0
                duration: 500
            }
        }
    }

    Dialog {
        id: dialog

        anchors.centerIn: parent
        title: qsTr("Do you want to remove the task?")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
    }

    ParallelAnimation {
        id: hideTasksList

        NumberAnimation {
            target: tasksList
            property: "contentY"
            duration: 500
            to: tasksList.height
        }
        NumberAnimation {
            target: tasksList
            property: "opacity"
            duration: 500
            to: 0
        }

   }

    ParallelAnimation {
        id: showTasksList

        NumberAnimation {
            target: tasksList
            property: "contentY"
            duration: 500
            to: 0
        }
        NumberAnimation {
            target: tasksList
            property: "opacity"
            duration: 500
            to: 1
        }

    }

   Connections {
       target: hideTasksList
       function onFinished() { tasksList.visible = false }
   }

   Connections {
       target: showTasksList
       function onStarted() { tasksList.visible = true }
   }
}
