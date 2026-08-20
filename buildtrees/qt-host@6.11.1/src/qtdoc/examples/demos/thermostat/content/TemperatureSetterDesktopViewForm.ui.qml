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
import QtQuick.Effects
import Thermostat
import ThermostatCustomControls

Pane {
    id: root

    required property ScheduleViewForm scheduleViewRoot
    property alias saveButton: saveButton
    property alias cancelButton: cancelButton
    property int currentMode: 2

    width: 1010
    height: 330
    padding: 0

    background: Rectangle {
        color: Constants.accentColor
        radius: 12
    }

    Row {
        id: row1
        width: root.width
        height: 70
        spacing: 80
        anchors.topMargin: 20
        anchors.leftMargin: 20

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 24

            Item {
                width: internal.iconSize
                height: internal.iconSize

                Image {
                    id: icon
                    source: "images/temperature.svg"
                    sourceSize.width: internal.iconSize
                    sourceSize.height: internal.iconSize
                }

                MultiEffect {
                    anchors.fill: icon
                    source: icon
                    colorization: 1
                    colorizationColor: Constants.iconColor
                }
            }

            Label {
                font.pixelSize: internal.fontSize
                font.weight: 600
                font.family: "Titillium Web"
                text: qsTr("Set Temperature :")
                color: Constants.primaryTextColor
            }
        }

        CustomTextField {
            id: customTextField
            anchors.verticalCenter: parent.verticalCenter
            text: slider.value
            Connections {
                function onAccepted() {
                    slider.value = +customTextField.text
                }
            }
        }

        CustomSlider {
            id: slider
            value: root.scheduleViewRoot.currentTemp
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            Connections {
                function onValueChanged() {
                    root.scheduleViewRoot.currentTemp = slider.value
                }
            }
        }
    }

    Column {
        anchors.top: row1.bottom
        anchors.left: parent.left
        anchors.topMargin: 20
        anchors.leftMargin: 20
        spacing: 50
        Row {
            id: row
            spacing: 60
            Label {
                font.pixelSize: 24
                font.weight: 600
                font.family: "Titillium Web"
                text: qsTr("Mode")
                anchors.verticalCenter: parent.verticalCenter
                color: Constants.primaryTextColor
            }

            Repeater {
                model: [qsTr("Heating"), qsTr("Cooling"), qsTr("Auto")]
                CustomRadioButton {
                    id: radioButton
                    required property string modelData
                    required property int index

                    text: modelData
                    indicatorSize: 20
                    checked: root.scheduleViewRoot.currentMode === index
                    Connections {
                        function onClicked() {
                            root.scheduleViewRoot.currentMode = radioButton.index
                        }
                    }
                }
            }
        }

        Row {
            spacing: 24

            Label {
                font.pixelSize: 24
                font.weight: 600
                font.family: "Titillium Web"
                text: qsTr("Repeat")
                anchors.verticalCenter: parent.verticalCenter
                color: Constants.primaryTextColor
            }

            Repeater {
                model: [qsTr("Mon"), qsTr("Tue"), qsTr("Wed"), qsTr(
                        "Thu"), qsTr("Fri"), qsTr("Sat"), qsTr("Sun")]

                CustomRoundButton {
                    id: roundButton
                    required property int index
                    required property string modelData

                    text: modelData
                    width: Constants.isSmallDesktopLayout ? 80 : 90
                    height: 50
                    radius: 12
                    font.pixelSize: 24
                    checked: root.scheduleViewRoot.selectedDays[index]
                    Connections {
                        function onClicked() {
                            root.scheduleViewRoot.selectedDays[roundButton.index]
                                    = !root.scheduleViewRoot.selectedDays[roundButton.index]
                        }
                    }
                }
            }
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 20
            spacing: 24

            CustomRoundButton {
                id: cancelButton

                width: 120
                height: 48
                text: qsTr("Cancel")
                radius: 12
                contentColor: "#2CDE85"
                checkable: false
                font.pixelSize: 14
            }

            CustomRoundButton {
                id: saveButton
                width: 120
                height: 48
                text: qsTr("Save")
                radius: 12
                contentColor: "#2CDE85"
                checkable: false
                font.pixelSize: 14
            }
        }
    }

    QtObject {
        id: internal
        property int fontSize: 24
        property int iconSize: 34
    }
}
