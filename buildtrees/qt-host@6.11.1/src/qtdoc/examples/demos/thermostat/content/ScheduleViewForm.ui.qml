// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Thermostat
import ThermostatCustomControls

Pane {
    id: root

    property int currentRoomIndex: 0
    required property list<Room> roomsList
    property string currentDate
    property list<int> selectedHours: [0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    property list<int> selectedDays: [0,0,0,0,0,0,0]
    property int currentMode: 2
    property int currentTemp: 10
    property alias scheduleViewRootForScrollView: scrollView.scheduleViewRoot
    property alias scheduleViewRootForSwipView: swipeView.scheduleViewRoot

    background: Rectangle {
        anchors.fill: parent
        color: Constants.backgroundColor
    }

    Column {
        id: title

        width: parent.width

        Label {
            id: header

            text: qsTr("Time Schedule")
            font.pixelSize: 48
            font.weight: 600
            font.family: "Titillium Web"
            color: Constants.primaryTextColor
            elide: Text.ElideRight
        }

        RowLayout {
            id: label

            width: parent.width
            Label {
                text: qsTr("Adjust a timer for specific room temperature")
                font.pixelSize: 24
                font.weight: 600
                font.family: "Titillium Web"
                color: Constants.accentTextColor
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            CustomComboBox {
                id: customComboBox
                roomsList: root.roomsList
                currentIndex: root.currentRoomIndex
                Connections {
                    function onCurrentIndexChanged() {
                        root.currentRoomIndex = customComboBox.currentIndex
                    }
                }
            }
        }
    }

    ScheduleStackView {
        id: scrollView

        anchors.top: title.bottom
        anchors.topMargin: 12
        anchors.leftMargin: 28

        width: internal.contentWidth
        height: internal.contentHeight

        roomsList: root.roomsList
        currentRoomIndex: root.currentRoomIndex
    }

    ScheduleSwipeView {
        id: swipeView

        anchors.top: title.bottom
        roomsList: root.roomsList

        width: internal.contentWidth
        height: internal.contentHeight

        visible: false
        currentRoomIndex: root.currentRoomIndex
        Connections {
            function onCurrentRoomIndexChanged() {
                root.currentRoomIndex = swipeView.currentRoomIndex
            }
        }
    }

    QtObject {
        id: internal
        readonly property int contentHeight: root.height - title.height
                                             - root.topPadding - root.bottomPadding
        readonly property int contentWidth: root.width - root.rightPadding - root.leftPadding
    }

    states: [
        State {
            name: "desktopLayout"
            when: Constants.isBigDesktopLayout || Constants.isSmallDesktopLayout
            PropertyChanges {
                target: root
                leftPadding: 20
            }
            PropertyChanges {
                target: header
                font.pixelSize: 48
                font.weight: 600
                font.family: "Titillium Web"
                visible: true
            }
            PropertyChanges {
                target: label
                visible: true
            }
            PropertyChanges {
                target: scrollView
                visible: true
            }
            PropertyChanges {
                target: swipeView
                visible: false
            }
        },
        State {
            name: "mobileLayout"
            when: Constants.isMobileLayout
            PropertyChanges {
                target: root
                leftPadding: 16
                rightPadding: 16
            }
            PropertyChanges {
                target: header
                font.pixelSize: 24
                font.weight: 600
                font.family: "Titillium Web"
                visible: true
            }
            PropertyChanges {
                target: label
                visible: false
            }
            PropertyChanges {
                target: scrollView
                visible: false
            }
            PropertyChanges {
                target: swipeView
                visible: true
            }
        },
        State {
            name: "smallLayout"
            when: Constants.isSmallLayout
            PropertyChanges {
                target: root
                leftPadding: 15
                rightPadding: 15
            }
            PropertyChanges {
                target: header
                visible: false
            }
            PropertyChanges {
                target: label
                visible: false
            }
            PropertyChanges {
                target: scrollView
                visible: false
            }
            PropertyChanges {
                target: swipeView
                visible: true
            }
        }
    ]
}
