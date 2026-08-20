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

Pane {
    id: root

    property alias calendar: monthGrid
    property alias calendarYear: monthGrid.year
    property alias monthSelector: monthSelector
    property alias months: monthSelector.model
    property alias years: yearSelector.model
    property alias currentMonth: monthSelector.currentIndex
    property alias currentYear: yearSelector.currentIndex
    property alias monthSelectorText: monthSelector.currentText
    property alias yearSelector: yearSelector
    property alias nextYear: nextYear
    property alias nextMonth: nextMonth
    property alias previousYear: previousYear
    property alias previousMonth: previousMonth
    property alias cancel: cancel
    property alias ok: ok

    property date selectedDate

    implicitWidth: 360
    implicitHeight: 460
    padding: 0

    ColumnLayout {
        anchors.fill: parent
        spacing: 34

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.topMargin: 20
            Layout.leftMargin: 10
            Layout.rightMargin: 10

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Image {
                    source: "images/Before_Icon.svg"

                    TapHandler {
                        id: previousMonth
                    }
                }

                ComboBox {
                    id: monthSelector

                    Layout.fillWidth: true
                    model: root.months
                    currentIndex: monthGrid.month
                }

                Image {
                    source: "images/Next_Icon.svg"

                    TapHandler {
                        id: nextMonth
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Image {
                    source: "images/Before_Icon.svg"

                    TapHandler {
                        id: previousYear
                    }
                }

                ComboBox {
                    id: yearSelector

                    Layout.fillWidth: true
                    model: root.years
                }

                Image {
                    source: "images/Next_Icon.svg"

                    TapHandler {
                        id: nextYear
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            DayOfWeekRow {
                Layout.fillWidth: true

                locale: monthGrid.locale
                delegate: Label {
                    required property string narrowName
                    color: Constants.secondaryColor
                    text: narrowName
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            MonthGrid {
                id: monthGrid

                Layout.fillWidth: true
                Layout.fillHeight: true

                delegate: Label {
                    id: monthGridDelegate
                    required property var model

                    color: Constants.secondaryColor
                    text: model.day
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    background: Rectangle {
                        anchors.centerIn: parent
                        implicitWidth: 40
                        implicitHeight: 40
                        radius: 20
                        color: "#41CD52"
                        visible: +root.selectedDate === +monthGridDelegate.model.date
                    }
                }
            }
        }

        Row {
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            Layout.rightMargin: 24
            Layout.bottomMargin: 18

            spacing: 32

            Label {
                text: qsTr("Cancel")
                color: "#41CD52"
                font.pixelSize: AppSettings.fontSize - 2
                leftInset: -4
                rightInset: -4

                background: Rectangle {
                    radius: 4
                    opacity: 0.3
                    color: cancel.pressed ? "#41CD52" : "transparent"
                }

                TapHandler {
                    id: cancel
                }
            }

            Label {
                text: qsTr("OK")
                color: "#41CD52"
                font.pixelSize: AppSettings.fontSize - 2
                leftInset: -4
                rightInset: -4

                background: Rectangle {
                    radius: 4
                    opacity: 0.3
                    color: ok.pressed ? "#41CD52" : "transparent"
                }

                TapHandler {
                    id: ok
                }
            }
        }
    }
}
