// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ToDoList

SwipeDelegate {
    id: root

    required property int index
    required property bool done
    required property bool highlight
    required property string name
    required property string date
    required property string time
    required property int task_id
    required property ListModel model
    required property bool isEditMode

    property alias tasksListArea: taskTapHandler
    signal taskToRemove(taskData: var)

    Drag.active: mouseArea.pressed
    Drag.hotSpot.x: root.width / 2
    Drag.hotSpot.y: root.height / 2

    implicitHeight: 60
    z: mouseArea.pressed ? 1 : 0
    opacity: root.done ? 0.3 : 1
    swipe.enabled: !root.done

    function moveElement(isDone: bool) {
        if (isDone) {
            model.move(index, model.count - 1, 1)}
        else {
            model.move(index, 0, 1)
        }
    }

    function highlightTask() {
        root.highlight = !root.highlight
        if (root.highlight && !root.done) {
            model.move(index, 0, 1)
        }
        Database.updateTaskHighlight(root.task_id, root.highlight ? 1 : 0)
        swipe.close()
    }

    function startTaskRemoving() {
        taskToRemove({idx: index, id: root.task_id})
        swipe.close()
    }

    function removeElement() {
        Database.deleteTask(root.task_id)
        model.remove(index)
    }

    background: Pane {
        opacity: root.isEditMode ? 0 : 1
    }

    contentItem: RowLayout {
        anchors.top: parent.top
        spacing: 16
        width: parent.width

        Image {
            id: dragIcon

            source: Constants.isDarkModeActive ? "images/Drag_Icon.svg" : "images/Drag_Icon_Dark.svg"
            visible: false
        }

        CheckBox {
            id: checkbox

            checked: root.done
            onClicked: {
                root.model.set(root.index, {"done": checked ? 1 : 0})
                itemCheckedAnim.start()
                Database.updateTaskState(root.task_id, checked ? 1 : 0)
            }
        }

        Column {
            id: column

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 8

            RowLayout {
                spacing: 5
                width: parent.width
                Layout.alignment: Qt.AlignVCenter

                Image {
                    source: "images/Star_Icon.svg"
                    visible: root.highlight
                }

                Label {
                    horizontalAlignment: Text.AlignLeft
                    Layout.fillWidth: true
                    text: root.name
                    font.bold: true
                    font.pixelSize: AppSettings.fontSize
                    color: root.highlight ? "#41CD52" : Constants.secondaryColor
                    elide: Text.ElideRight
                }

                Image {
                    id: trashIcon
                    source: "images/Red_Trash.svg"
                    visible: false

                    TapHandler {
                        id: trashButton
                        onTapped: root.startTaskRemoving()
                    }
                }
            }

            Row {
                id: dateTimeLabels

                leftPadding: 8
                spacing: 20

                CustomLabel {
                    id: dateLabel

                    text: root.date
                    font.bold: false
                    backgroundColor: "#41CD52"
                    backgroundOpacity: 0.1
                    tapHandler.enabled: false
                }

                CustomLabel {
                    id: timeLabel

                    text: root.time
                    font.bold: false
                    color: !Constants.isDarkModeActive ? "#53586B" : "#CECFD5"
                    backgroundBorder.color: "#9D9FAA"
                    visible: root.time
                    tapHandler.enabled: false
                }
            }
        }
    }

    swipe.left: SwipeDelegateContent {
        id: flagLabel

        color: "#41CD52"
        opacity: root.pressed && root.swipe.position > 0
        iconSource: "images/White_Star.svg"
        labelText: qsTr("Flag")
    }

    swipe.right: SwipeDelegateContent {
        id: deleteLabel

        anchors.right: root.right
        opacity: root.pressed && root.swipe.position < 0
        color: "#CD084F"
        iconSource: "images/White_Trash.svg"
        labelText: qsTr("Delete")
    }

    swipe.onOpened: {
        if (swipe.position < 0) {
            root.startTaskRemoving()
        } else {
            root.highlightTask()
        }
    }

    swipe.transition: Transition {
        SmoothedAnimation { velocity: 3; easing.type: Easing.InOutCubic }
    }

    TapHandler {
        id: taskTapHandler

        longPressThreshold: 2
    }

    Rectangle {
        id: dropAreaBackground

        anchors.fill: parent
        opacity: 0.2
        radius: 10
        color: mouseArea.pressed ? "#41CD52" : "white"
        visible: false
    }

    MouseArea {
        id: mouseArea

        enabled: root.isEditMode && !taskTapHandler.enabled
        anchors.fill: parent
        anchors.rightMargin: 80
        drag.target: parent
        drag.axis: Drag.YAxis

        //Properties needed to store start position of the item.
        //It prevents the element to be placed in wrong position.
        //When item is dropped outside droparea it goes back to the startup positions.
        property int startX: 0
        property int startY: 0
        property int startIndex: 0

        onPressed: {
            startX = root.x
            startY = root.y
            startIndex = root.index
            dropAreaBackground.visible = true
        }

        onReleased: {
            parent.Drag.drop()
            dropAreaBackground.visible = false
            if (startIndex === root.index) {
                root.x = startX
                root.y = startY
            }
        }

        DropArea {
            id: dropArea

            enabled: root.isEditMode && !taskTapHandler.enabled
            anchors.fill: parent

            onEntered: dropAreaBackground.visible = true
            onExited: dropAreaBackground.visible = false

            onDropped: {
                dropAreaBackground.visible = false
                root.model.move(drag.source.DelegateModel.itemsIndex, root.index, 1)
            }
        }
    }

    SequentialAnimation {
        id: itemCheckedAnim

        NumberAnimation {
            target: checkbox
            properties: "scale"
            from: 1
            to: 1.5
            duration: 250
        }
        NumberAnimation {
            target: checkbox
            properties: "scale"
            from: 1.5
            to: 1
            duration: 250
        }

        onFinished: {
            if (!AppSettings.removeDoneTasks) {
                root.moveElement(checkbox.checked)
            } else if (AppSettings.removeDoneTasks && checkbox.checked) {
                root.removeElement()
            }
        }
    }

    states: [
        State {
            name: "normalMode"
            when: !root.isEditMode
            PropertyChanges { root.swipe.enabled: true }
            PropertyChanges { dragIcon.visible: false }
            PropertyChanges { dateTimeLabels.visible: true }
            PropertyChanges { trashIcon.visible: false }
            PropertyChanges { taskTapHandler.enabled: true }
            PropertyChanges { checkbox.visible: true }
        },
        State {
            name: "editMode"
            when: root.isEditMode && !checkbox.checked
            PropertyChanges { root.swipe.enabled: false }
            PropertyChanges { dragIcon.visible: true }
            PropertyChanges { dateTimeLabels.visible: false }
            PropertyChanges { trashIcon.visible: true }
            PropertyChanges { taskTapHandler.enabled: false }
            PropertyChanges { checkbox.visible: false }
        }
    ]
}
