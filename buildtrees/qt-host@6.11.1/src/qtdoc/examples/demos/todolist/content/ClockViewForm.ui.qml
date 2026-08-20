// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ToDoList
import CustomControls

Pane {
    id: root

    implicitWidth: 330
    implicitHeight: 520
    padding: 0

    property alias timeInput: timeInput
    property alias cancel: cancel
    property alias accept: accept
    property alias clock: clock
    property alias isHourClock: clock.isHourClock
    property alias angle: clock.angle
    property alias clockHour: clock.hour
    property alias clockMin: clock.min
    property alias hourTextField: timeInput.hourTextField
    property alias minTextField: timeInput.minTextField
    property alias minText: timeInput.minText
    property alias hourText: timeInput.hourText
    property alias amPmText: timeInput.amPm

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Label {
            text: qsTr("Select time")
            color: Constants.secondaryColor
            font.pixelSize: AppSettings.fontSize
        }

        TimeInput {
            id: timeInput
        }

        Clock {
            id: clock
            Layout.alignment: Qt.AlignHCenter
        }

        Row {
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            spacing: 32

            Label {
                text: qsTr("Cancel")
                color: "#41CD52"
                font.pixelSize: AppSettings.fontSize
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
                font.pixelSize: AppSettings.fontSize
                leftInset: -4
                rightInset: -4

                background: Rectangle {
                    radius: 4
                    opacity: 0.3
                    color: accept.pressed ? "#41CD52" : "transparent"
                }

                TapHandler {
                    id: accept
                }
            }
        }
    }
}
