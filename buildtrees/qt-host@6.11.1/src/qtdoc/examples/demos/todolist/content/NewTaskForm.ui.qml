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

    property alias backButton: header.backButton
    property alias acceptButton: header.acceptButton
    property alias addButton: addButton
    property alias calendar: calendarView
    property alias clock: clock
    property alias isHourClock: clock.isHourClock
    property alias hourText: clock.hourText
    property alias minText: clock.minText
    property alias amPmText: clock.amPmText
    property alias calendarButton: dueDate.iconButton
    property alias clockButton: dueTime.iconButton
    property alias titleText: title.text
    property alias dueDateText: dueDate.text
    property alias datePlaceholderText: dueDate.placeholderText
    property alias timePlaceholderText: dueTime.placeholderText
    property alias dueTimeText: dueTime.text
    property alias notes: notes.text
    property alias selectedDate: calendarView.selectedDate

    property bool isEditMode: false
    property int currentTaskIndex: -1
    property var sourceModel

    padding: 0

    header: NavBar {
        id: header

        visible: !background.visible
        titleText: root.isEditMode ? qsTr("Edit task") : qsTr("New task")
        previousPageTitle: qsTr("Home")
        acceptButtonVisible: true
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 20

        CustomTextField {
            id: title

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 80
            Layout.minimumHeight: 60

            headerText: qsTr("Title")
            placeholderText: qsTr("Task name")
            validator: RegularExpressionValidator {
                regularExpression: /^(?!\s*$).+/
            }
        }

        CustomTextField {
            id: dueDate

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 80
            Layout.minimumHeight: 60

            headerText: qsTr("Due date")
            iconSource: "images/Calendar_Icon.svg"
            inputMethodHints: Qt.ImhDate | Qt.ImhPreferNumbers
        }

        CustomTextField {
            id: dueTime

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 80
            Layout.minimumHeight: 60

            headerText: qsTr("Due time")
            iconSource: "images/Clock_Icon.svg"
            inputMethodHints: Qt.ImhTime
        }

        CustomTextArea {
            id: notes

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 40
        }
        Button {
            id: addButton

            Layout.fillWidth: true
            Layout.minimumHeight: 20
            Layout.preferredHeight: 56
            text: root.isEditMode ? qsTr("Update task") : qsTr("Add task")

            palette.button: Constants.isDarkModeActive ? "#30D158" : "#34C759"
            palette.highlight: Constants.isDarkModeActive ? "#30DB5B" : "#248A3D"
        }
    }

    Rectangle {
        id: background

        anchors.fill: parent
        color: "black"
        opacity: 0.75
        visible: calendarView.visible || clock.visible

        MouseArea {
            anchors.fill: background
            preventStealing: true
        }
    }

    CalendarView {
        id: calendarView

        anchors.centerIn: parent
        visible: false
    }

    ClockView {
        id: clock

        anchors.centerIn: parent
        visible: false
    }
}
