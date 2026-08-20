// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

ItemDelegate {
    id: root
    width: parent.width
    checkable: true

    required property int index
    required property int hour
    required property int minute
    required property int day
    required property int month
    required property int year
    required property bool activated
    required property string label
    required property bool repeat
    required property list<var> daysToRepeat

    onClicked: ListView.view.currentIndex = index

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            ColumnLayout {
                id: dateColumn

                readonly property date alarmDate: new Date(
                    root.year, root.month - 1, root.day, root.hour, root.minute)

                Label {
                    id: timeLabel
                    font.pixelSize: (Qt.application as Application).font.pixelSize * 2
                    text: dateColumn.alarmDate.toLocaleTimeString(root.locale, Locale.ShortFormat)
                }
                RowLayout {
                    Label {
                        id: dateLabel
                        text: dateColumn.alarmDate.toLocaleDateString(root.locale, Locale.ShortFormat)
                    }
                    Label {
                        id: alarmAbout
                        text: "⸱ " + root.label
                        visible: root.label.length > 0 && !root.checked
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
            Switch {
                checked: root.activated
                Layout.alignment: Qt.AlignTop
                onClicked: root.activated = checked
            }
        }
        CheckBox {
            id: alarmRepeat
            text: qsTr("Repeat")
            checked: root.repeat
            visible: root.checked
            onToggled: root.repeat = checked
        }
        Flow {
            visible: root.checked && root.repeat
            Layout.fillWidth: true

            Repeater {
                id: dayRepeater
                model: root.daysToRepeat
                delegate: RoundButton {
                    required property int dayOfWeek
                    required property bool repeat
                    text: Qt.locale().dayName(dayOfWeek, Locale.NarrowFormat)
                    flat: true
                    checked: repeat
                    checkable: true
                    Material.background: checked ? Material.accent : "transparent"
                    onToggled: repeat = checked
                }
            }
        }

        TextField {
            id: alarmDescriptionTextField
            placeholderText: qsTr("Enter description here")
            cursorVisible: true
            visible: root.checked
            text: root.label
            onTextEdited: root.label = text
        }
        Button {
            id: deleteAlarmButton
            text: qsTr("Delete")
            visible: root.checked
            onClicked: root.ListView.view.model.remove(root.ListView.view.currentIndex, 1)
        }
    }
}
