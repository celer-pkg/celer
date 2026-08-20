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

    required property list<Room> roomsList

    background: Rectangle {
        anchors.fill: parent
        color: Constants.backgroundColor
    }

    Column {
        id: title

        width: internal.contentWidth

        RowLayout {
            id: smallHeading
            width: parent.width

            Label {
                id: smallHeadingLabel
                Layout.fillWidth: true

                text: qsTr("Welcome")
                font: Constants.desktopTitleFont
                color: Constants.primaryTextColor
                elide: Text.ElideRight
            }


            CustomSwitch {
                id: activeToggle

                checked: AppSettings.isThermostatActive
                Connections {
                    function onClicked() {
                        AppSettings.isThermostatActive = activeToggle.checked
                    }
                }
            }
        }

        RowLayout {
            id: largeHeading

            width: parent.width

            Label {
                Layout.fillWidth: true
                text: qsTr("Here's the list of your Rooms at Home")
                font.pixelSize: 24
                font.weight: 600
                font.family: "Titillium Web"
                color: Constants.accentTextColor
                elide: Text.ElideRight
            }

            Rectangle {
                implicitWidth: 120
                implicitHeight: 32
                radius: 8
                color: "transparent"
                border.color: Constants.accentTextColor

                CustomRoundButton {
                    id: otherButton
                    x: !checked ? 0 : 50
                    width: 70
                    height: 32
                    radius: 8

                    font.pixelSize: 10
                    font.family: "Inter"
                    font.weight: 600
                    text: qsTr("At home")
                    checked: AppSettings.isThermostatActive
                    Connections {
                        function onClicked() {
                            AppSettings.isThermostatActive = otherButton.checked
                        }
                    }
                }
            }
        }
    }

    RoomsScrollView {
        id: scrollView

        anchors.top: title.bottom
        anchors.topMargin: 10

        width: internal.contentWidth
        height: internal.contentHeight
        gridWidth: internal.contentWidth
        gridHeight: internal.contentHeight

        delegatePreferredWidth: internal.delegatePreferredWidth
        delegatePreferredHeight: internal.delegatePreferredHeight

        columns: root.width < 1140 ? 1 : 2
        roomsList: root.roomsList
    }

    RoomsSwipeView {
        id: swipeView

        anchors.top: title.bottom
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.right: parent.right

        width: internal.contentWidth
        height: internal.contentHeight
        delegatePreferredHeight: internal.delegatePreferredHeight
        delegatePreferredWidth: internal.delegatePreferredWidth

        roomsList: root.roomsList
        visible: false
    }

    QtObject {
        id: internal

        readonly property int contentHeight: root.height - title.height
                                             - root.topPadding - root.bottomPadding
        readonly property int contentWidth: root.width - root.rightPadding - root.leftPadding
        property int delegatePreferredHeight: 276
        property int delegatePreferredWidth: 530
    }

    states: [
        State {
            name: "desktopLayout"
            when: Constants.isBigDesktopLayout || Constants.isSmallDesktopLayout
            PropertyChanges {
                target: smallHeadingLabel
                text: qsTr("Welcome")
                font: Constants.desktopTitleFont
            }
            PropertyChanges {
                target: largeHeading
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
            PropertyChanges {
                target: internal
                delegatePreferredHeight: 276
                delegatePreferredWidth: 530
            }
            PropertyChanges {
                target: root
                leftPadding: 27
            }
            PropertyChanges {
                target: activeToggle
                visible: false
            }
        },
        State {
            name: "mobileLayout"
            when: Constants.isMobileLayout
            PropertyChanges {
                target: smallHeadingLabel
                text: qsTr("Rooms")
                font: Constants.mobileTitleFont
            }
            PropertyChanges {
                target: largeHeading
                visible: false
            }
            PropertyChanges {
                target: scrollView
                visible: true
            }
            PropertyChanges {
                target: swipeView
                visible: false
            }
            PropertyChanges {
                target: internal
                delegatePreferredHeight: 177
                delegatePreferredWidth: 306
            }
            PropertyChanges {
                target: root
                leftPadding: 27
            }
            PropertyChanges {
                target: activeToggle
                visible: true
            }
        },
        State {
            name: "smallLayout"
            when: Constants.isSmallLayout
            PropertyChanges {
                target: smallHeadingLabel
                text: qsTr("Rooms")
                font: Constants.smallTitleFont
            }
            PropertyChanges {
                target: largeHeading
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
            PropertyChanges {
                target: internal
                delegatePreferredHeight: 177
                delegatePreferredWidth: 340
            }
            PropertyChanges {
                target: root
                leftPadding: 11
            }
            PropertyChanges {
                target: activeToggle
                visible: true
            }
        }
    ]
}
