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
import QtQml
import ToDoList

Pane {
    id: root

    implicitHeight: 100
    leftPadding: 0
    rightPadding: 0

    property alias enterButton: addQuickTask
    property alias newQuickTask: taskTextField
    property alias taskTitle: taskTextField.text

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        TextField {
            id: taskTextField

            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.alignment: Qt.AlignVCenter

            implicitHeight: 32
            placeholderText: qsTr("Add new task")
            placeholderTextColor: "#41CD52"

            rightPadding: 40

            validator: RegularExpressionValidator {
                regularExpression: /^(?!\s*$).+/
            }

            Image {
                source: "images/Enter_Icon.svg"
                anchors.right: taskTextField.right
                anchors.rightMargin: 12
                anchors.verticalCenter: taskTextField.verticalCenter
                visible: taskTextField.text

                TapHandler {
                    id: addQuickTask
                }
            }
        }

        Rectangle {
            id: separator

            implicitHeight: 1
            Layout.fillWidth: true
            color: Constants.secondaryColor
            opacity: 0.25
        }
    }
}
