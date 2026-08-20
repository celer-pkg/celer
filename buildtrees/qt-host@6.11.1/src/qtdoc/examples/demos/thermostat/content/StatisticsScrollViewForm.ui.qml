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
import ThermostatCustomControls
import Thermostat

ScrollView {
    id: scrollView

    required property Room room

    property bool isOneColumn: false
    property bool isBackgroundVisible: false
    property int statisticsChartHeight: 647
    property int statisticsChartWidth: 1098
    property int delegateHeight: 182
    property int delegateWidth: 350

    clip: true
    bottomPadding: 10
    contentWidth: availableWidth

    background: Rectangle {
        color: Constants.accentColor
        visible: scrollView.isBackgroundVisible
        radius: 12
    }

    GridLayout {
        id: grid

        width: scrollView.width
        height: scrollView.height
        anchors.horizontalCenter: parent.horizontalCenter

        columns: scrollView.isOneColumn ? 1 : 3
        rows: scrollView.isOneColumn ? 8 : 1

        columnSpacing: 24
        rowSpacing: scrollView.isOneColumn ? 12 : 24

        Pane {
            id: statistics

            Layout.columnSpan: scrollView.isOneColumn ? 1 : 3
            Layout.rowSpan: scrollView.isOneColumn ? 5 : 1

            Layout.preferredHeight: scrollView.statisticsChartHeight
            Layout.preferredWidth: scrollView.statisticsChartWidth
            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                radius: 12
                color: Constants.accentColor
            }

            StatisticsChart {
                id: statisticsChart

                anchors.fill: parent
                anchors.horizontalCenter: parent.horizontalCenter
                energyValues: scrollView.room.energyStats
                tempValues: scrollView.room.tempStats
            }
        }

        TemperatureInfo {
            id: tempInfo
            Layout.preferredHeight: scrollView.delegateHeight
            Layout.preferredWidth: scrollView.delegateWidth
            Layout.alignment: Qt.AlignHCenter

            temperatureValues: scrollView.room.tempStats
        }

        HumidityInfo {
            id: humidityInfo
            Layout.preferredHeight: scrollView.delegateHeight
            Layout.preferredWidth: scrollView.delegateWidth
            Layout.alignment: Qt.AlignHCenter

            humidityValues: scrollView.room.humidityStats
        }

        EnergyInfo {
            id: energyInfo
            Layout.preferredHeight: scrollView.delegateHeight
            Layout.preferredWidth: scrollView.delegateWidth
            Layout.alignment: Qt.AlignHCenter

            energyValues: scrollView.room.energyStats
        }
    }

    states: [
        State {
            name: "bigDesktopLayout"
            when: Constants.isBigDesktopLayout
            PropertyChanges {
                target: statistics
                leftPadding: 53
                rightPadding: 53
                topPadding: 23
                bottomPadding: 43
            }
            PropertyChanges {
                target: scrollView
                isBackgroundVisible: false
                delegateWidth: 350
                delegateHeight: 182
                statisticsChartWidth: grid.width
                statisticsChartHeight: 647
            }
            PropertyChanges {
                target: grid
                width: 1100
            }
        },
        State {
            name: "smallDesktopLayout"
            when:  Constants.isSmallDesktopLayout
            PropertyChanges {
                target: statistics
                leftPadding: 53
                rightPadding: 53
                topPadding: 23
                bottomPadding: 43
            }
            PropertyChanges {
                target: scrollView
                isBackgroundVisible: false
                delegateWidth: 290
                delegateHeight: 182
                statisticsChartWidth: grid.width
                statisticsChartHeight: 541
            }
            PropertyChanges {
                target: grid
                width: 918
            }
        },
        State {
            name: "mobileLayout"
            when: Constants.isMobileLayout
            PropertyChanges {
                target: statistics
                leftPadding: 0
                rightPadding: 0
                topPadding: 0
                bottomPadding: 0
            }
            PropertyChanges {
                target: scrollView
                isBackgroundVisible: false
                delegateWidth: 327
                delegateHeight: 110
                statisticsChartWidth: 346
                statisticsChartHeight: 383
            }
        },
        State {
            name: "smallLayout"
            when: Constants.isSmallLayout
            PropertyChanges {
                target: statistics
                leftPadding: 0
                rightPadding: 0
                topPadding: 0
                bottomPadding: 0
            }
            PropertyChanges {
                target: scrollView
                isBackgroundVisible: true
                delegateWidth: 332
                delegateHeight: 90
                statisticsChartWidth: 420
                statisticsChartHeight: 240
            }
        }
    ]
}
