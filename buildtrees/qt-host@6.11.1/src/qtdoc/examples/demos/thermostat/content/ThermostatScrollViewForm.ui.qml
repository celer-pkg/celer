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

ScrollView {
    id: root

    clip: true
    bottomPadding: 10
    contentWidth: availableWidth

    required property Room room

    property bool isOneColumn
    property int thermostatControlHeight
    property int thermostatControlWidth
    property int delegateHeight
    property int delegateWidth

    background: Rectangle {
        color: Constants.isSmallLayout ? Constants.accentColor : "transparent"
        radius: 12
    }

    GridLayout {
        id: grid

        width: root.width
        height: root.height
        anchors.horizontalCenter: parent.horizontalCenter
        columns: root.isOneColumn ? 1 : 3
        rows: root.isOneColumn ? 10 : 1
        columnSpacing: 24
        rowSpacing: root.isOneColumn ? 12 : 24

        ThermostatSettings {
            id: thermoSettings

            Layout.columnSpan: root.isOneColumn ? 1 : 3
            Layout.rowSpan: root.isOneColumn ? 7 : 1
            Layout.preferredHeight: root.thermostatControlHeight
            Layout.preferredWidth: root.thermostatControlWidth
            Layout.alignment: Qt.AlignHCenter

            room: root.room

            Connections {
                target: thermoSettings
                function onIsActiveChanged() {
                    root.room.active = thermoSettings.isActive
                }
            }
        }

        TemperatureInfo {
            Layout.preferredHeight: root.delegateHeight
            Layout.preferredWidth: root.delegateWidth
            Layout.alignment: Qt.AlignHCenter

            temperatureValues: root.room.tempStats
        }

        HumidityInfo {
            Layout.preferredHeight: root.delegateHeight
            Layout.preferredWidth: root.delegateWidth
            Layout.alignment: Qt.AlignHCenter

            humidityValues: root.room.humidityStats
        }

        EnergyInfo {
            Layout.preferredHeight: root.delegateHeight
            Layout.preferredWidth: root.delegateWidth
            Layout.alignment: Qt.AlignHCenter

            energyValues: root.room.energyStats
        }
    }

    states: [
        State {
            name: "bigDesktopLayout"
            when: Constants.isBigDesktopLayout
            PropertyChanges {
                target: root
                thermostatControlHeight: 673
                thermostatControlWidth: grid.width
                delegateHeight: 182
                delegateWidth: 350
            }
            PropertyChanges {
                target: grid
                width: 1100
            }
        },
        State {
            name: "smallDesktopLayout"
            when: Constants.isSmallDesktopLayout
            PropertyChanges {
                target: root
                thermostatControlHeight: 673
                thermostatControlWidth: grid.width
                delegateHeight: 182
                delegateWidth: 290
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
                target: root
                delegateWidth: 327
                delegateHeight: 120
                thermostatControlHeight: 694
                thermostatControlWidth: 327
            }
        },
        State {
            name: "smallLayout"
            when: Constants.isSmallLayout
            PropertyChanges {
                target: root
                delegateWidth: 332
                delegateHeight: 90
                thermostatControlHeight: 250
                thermostatControlWidth: 400
            }
        }
    ]
}
