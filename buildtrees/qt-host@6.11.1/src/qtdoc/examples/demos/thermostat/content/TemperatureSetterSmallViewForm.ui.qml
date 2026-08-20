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

    width: 400

    topPadding: 16
    bottomPadding: 19

    background: Rectangle {
        color: Constants.accentColor
        radius: 12
    }

    Frame {
        id: frame
        height: 200
        width: 380
        anchors.horizontalCenter: parent.horizontalCenter

        ColumnLayout {
            anchors.fill: parent

            RowLayout {
                Layout.preferredWidth: parent.width

                Row {
                    id: row
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                    spacing: 10

                    Item {
                        width: 15
                        height: 15
                        anchors.verticalCenter: parent.verticalCenter

                        Image {
                            id: icon

                            source: "images/temperature.svg"
                            sourceSize.width: 15
                            sourceSize.height: 15
                        }

                        MultiEffect {
                            anchors.fill: icon
                            source: icon
                            colorization: 1
                            colorizationColor: Constants.iconColor
                        }
                    }

                    Label {
                        font.pixelSize: 14
                        font.weight: 600
                        font.family: "Titillium Web"
                        text: qsTr("Set Temperature :")
                        color: Constants.primaryTextColor
                    }
                }

                CustomTextField {
                    id: customTextField
                    Layout.preferredWidth: 102
                    Layout.preferredHeight: 40
                    font.pixelSize: 14
                    text: slider.value
                    Connections {
                        function onAccepted() {
                            slider.value = +customTextField.text
                        }
                    }
                }
            }

            Item {
                id: item1
                Layout.fillWidth: true
                Layout.fillHeight: true
                CustomSlider {
                    id: slider
                    width: parent.width
                    value: root.scheduleViewRoot.currentTemp
                    anchors.bottom: parent.bottom
                    Connections {
                        function onValueChanged() {
                            root.scheduleViewRoot.currentTemp = slider.value
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 0

                Label {
                    font.pixelSize: 14
                    font.weight: 600
                    font.family: "Titillium Web"
                    text: qsTr("Mode")
                    color: Constants.primaryTextColor
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.fillWidth: true
                    Layout.leftMargin: 50
                    spacing: 16
                    Repeater {
                        model: [qsTr("Heating"), qsTr("Cooling"), qsTr("Auto")]
                        CustomRadioButton {
                            id: radioButton
                            required property string modelData
                            required property int index
                            text: modelData
                            font.pixelSize: 14
                            indicatorSize: 14
                            topPadding: 0
                            bottomPadding: 0
                            checked: root.scheduleViewRoot.currentMode === index
                            Connections {
                                function onClicked() {
                                    root.scheduleViewRoot.currentMode = radioButton.index
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 0

                Label {
                    font.pixelSize: 14
                    font.weight: 600
                    font.family: "Titillium Web"
                    text: qsTr("Repeat")
                    color: Constants.primaryTextColor
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.fillWidth: true
                    Layout.leftMargin: 50
                    spacing: 16

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
        }
    }

    Row {
        anchors.top: frame.bottom
        anchors.right: frame.right
        anchors.topMargin: 16
        spacing: 17

        CustomRoundButton {
            id: cancelButton

            width: 78
            height: 38
            text: qsTr("Cancel")
            radius: 8
            contentColor: "#2CDE85"
            checkable: false
            font.pixelSize: 14
            display: AbstractButton.TextOnly
        }

        CustomRoundButton {
            id: saveButton

            width: 78
            height: 38
            text: qsTr("Save")
            radius: 8
            contentColor: "#2CDE85"
            checkable: false
            font.pixelSize: 14
            display: AbstractButton.TextOnly
        }
    }
}
