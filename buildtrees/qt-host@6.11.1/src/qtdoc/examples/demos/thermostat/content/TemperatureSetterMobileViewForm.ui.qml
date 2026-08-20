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
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Effects
import Thermostat
import ThermostatCustomControls

Pane {
    id: root

    required property ScheduleViewForm scheduleViewRoot
    property alias saveButton: saveButton
    property alias cancelButton: cancelButton

    width: 329
    height: 430

    bottomPadding: 10

    background: Rectangle {
        color: Constants.accentColor
        radius: 12
    }

    ColumnLayout {
        width: parent.width
        spacing: 15
        Row {
            spacing: 15

            Item {
                width: 24
                height: 24
                anchors.verticalCenter: parent.verticalCenter
                Image {
                    id: icon
                    source: "images/temperature.svg"
                    sourceSize.width: 24
                    sourceSize.height: 24
                }
                MultiEffect {
                    anchors.fill: icon
                    source: icon
                    colorization: 1
                    colorizationColor: Constants.iconColor
                }
            }

            Label {
                font.pixelSize: 18
                font.weight: 600
                font.family: "Titillium Web"
                text: qsTr("Set Temperature :")
                color: Constants.primaryTextColor
                anchors.verticalCenter: parent.verticalCenter
            }

            CustomTextField {
                id: customTextField
                width: 90
                height: 40
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 14
                text: slider.value
                Connections {
                    function onAccepted() {
                        slider.value = +customTextField.text
                    }
                }
            }
        }

        CustomSlider {
            id: slider
            Layout.fillWidth: true
            Layout.topMargin: 12
            value: root.scheduleViewRoot.currentTemp
            Connections {
                function onValueChanged() {
                    root.scheduleViewRoot.currentTemp = slider.value
                }
            }
        }

        Column {
            Label {
                font.pixelSize: 18
                font.weight: 600
                font.family: "Titillium Web"
                text: qsTr("Mode")
                color: Constants.primaryTextColor
            }

            Repeater {
                model: [qsTr("Heating"), qsTr("Cooling"), qsTr("Auto")]
                CustomRadioButton {
                    id: radioButton
                    required property string modelData
                    required property int index

                    text: modelData
                    font.pixelSize: 14
                    indicatorSize: 14
                    checked: root.scheduleViewRoot.currentMode === index
                    Connections {
                        function onClicked() {
                            root.scheduleViewRoot.currentMode = radioButton.index
                        }
                    }
                }
            }
        }

        ColumnLayout {
            spacing: 12
            Label {
                font.pixelSize: 18
                font.weight: 600
                font.family: "Titillium Web"
                text: qsTr("Repeat")
                color: Constants.primaryTextColor
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                spacing: 24
                Repeater {
                    id: repeater
                    model: [qsTr("MO"), qsTr("TU"), qsTr("WE"), qsTr(
                            "TH"), qsTr("FR"), qsTr("SA"), qsTr("SU")]

                    Label {
                        id: weekdayLabel
                        required property int index
                        required property string modelData
                        property bool checked: root.scheduleViewRoot.selectedDays[index]

                        text: modelData
                        Layout.fillWidth: true
                        height: 21
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        color: checked ? "#2CDE85" : Constants.primaryTextColor
                        TapHandler {
                            property Connections _: Connections {
                                function onTapped() {
                                    root.scheduleViewRoot.selectedDays[weekdayLabel.index]
                                            = !root.scheduleViewRoot.selectedDays[weekdayLabel.index]
                                }
                            }
                        }
                    }
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            CustomRoundButton {
                id: cancelButton

                Layout.preferredHeight: 45
                Layout.fillWidth: true
                text: qsTr("Cancel")
                radius: 12
                contentColor: "#2CDE85"
                checkable: false
                font.pixelSize: 14
            }

            CustomRoundButton {
                id: saveButton

                Layout.preferredHeight: 45
                Layout.fillWidth: true
                text: qsTr("Save")
                radius: 12
                contentColor: "#2CDE85"
                checkable: false
                font.pixelSize: 14
            }
        }
    }
}
