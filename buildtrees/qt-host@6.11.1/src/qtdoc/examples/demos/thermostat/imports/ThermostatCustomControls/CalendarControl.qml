// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import Thermostat

Popup {
    id: root

    width: Constants.isSmallLayout ? 250 : 360
    height: Constants.isSmallLayout ? 250 : 360
    modal: true

    padding: Constants.isSmallLayout ? 16 : 32
    anchors.centerIn: Overlay.overlay
    palette.shadow: AppSettings.isDarkTheme ? "#FFFFFF" : "#000000"

    readonly property date currentDate: {
        let now = new Date()
        // We don't want the time components
        return new Date(now.getFullYear(), now.getMonth(), now.getDate())
    }
    property date selectedDate: currentDate
    property int currentYear: currentDate.getFullYear()
    property int currentMonth: currentDate.getMonth()

    function nextMonth() {
        if (currentMonth === Calendar.December) {
            currentMonth = Calendar.January;
            ++currentYear;
        } else {
            ++currentMonth;
        }
    }

    function previousMonth() {
        if (currentMonth === Calendar.January) {
            currentMonth = Calendar.December;
            --currentYear;
        } else {
            --currentMonth;
        }
    }

    background: Rectangle {
        radius: 12
        color: Constants.accentColor
    }

    contentItem: Pane {
        anchors.fill: parent
        padding: root.padding

        background: Rectangle {
            color: "transparent"
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 24
            RowLayout {
                spacing: 12
                Label {
                    font.pixelSize: 14
                    font.weight: 700
                    font.family: "Titillium Web"
                    text: Qt.locale().monthName(root.currentMonth, Locale.LongFormat) + " " + root.currentYear
                    color: Constants.primaryTextColor
                    Layout.fillWidth: true
                }
                CustomRoundButton {
                    implicitWidth: 24
                    implicitHeight: 24
                    radius: 12
                    text: "<"
                    font.pixelSize: 12
                    display: AbstractButton.TextOnly
                    checkable: false
                    onClicked: root.previousMonth()
                }
                CustomRoundButton {
                    implicitWidth: 24
                    implicitHeight: 24
                    radius: 12
                    text: ">"
                    font.pixelSize: 12
                    display: AbstractButton.TextOnly
                    checkable: false
                    onClicked: root.nextMonth()
                }
            }

            DayOfWeekRow {
                locale: grid.locale

                Layout.fillWidth: true

                delegate: Text {
                    required property string narrowName
                    text: narrowName
                    font.pixelSize: 14
                    font.weight: 400
                    font.family: "Titillium Web"
                    color: Constants.accentTextColor

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            MonthGrid {
                id: grid

                month: root.currentMonth
                year: root.currentYear
                locale: Qt.locale()

                Layout.fillWidth: true
                Layout.fillHeight: true

                delegate: Rectangle {
                    id: monthGridDelegate
                    readonly property bool isSelected: {
                        return root.selectedDate.getFullYear() === date.getFullYear()
                                   && root.selectedDate.getMonth() === date.getMonth()
                                   && root.selectedDate.getDate() === date.getDate()
                    }
                    required property int day
                    required property int month
                    required property date date

                    radius: 8
                    width: 40
                    height: 40
                    color: !isSelected ? "transparent" : "#2CDE85"

                    Text {
                        anchors.centerIn: parent
                        text: monthGridDelegate.day
                        font.pixelSize: 14
                        font.family: "Inter"
                        opacity: monthGridDelegate.month === grid.month ? 1 : 0.5
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: monthGridDelegate.isSelected ? "#FFFFFF" : Constants.primaryTextColor
                    }
                }

                onPressed: date => {
                    root.selectedDate = date;
                }
            }
        }
    }
}
