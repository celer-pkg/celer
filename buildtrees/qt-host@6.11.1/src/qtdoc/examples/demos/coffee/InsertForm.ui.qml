// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root
    property alias continueButton: continueButton
    property alias cancelButton: cancelButton
    property alias dialog: dialog
    property string coffeeName: ""

    states: [
        State {
            name: "portrait"
            PropertyChanges {
                target: grid
                flow: GridLayout.TopToBottom
                rowSpacing: 20
                columns: 1
                rows: 4
            }
            PropertyChanges {
                target: cup
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: height / 1.16
                Layout.preferredHeight: root.height / 3
                Layout.row: 0
            }
            PropertyChanges {
                target: dialog
                Layout.row: 1
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: root.width / 1.12
                Layout.preferredHeight: root.height / 7
            }
            PropertyChanges {
                target: continueButton
                Layout.row: 2
                Layout.preferredWidth: root.width / 2.2
                Layout.preferredHeight: root.height / 14
            }
            PropertyChanges {
                target: cancelButton
                Layout.row: 3
                Layout.preferredWidth: root.width / 2.2
                Layout.preferredHeight: root.height / 14
            }
        },
        State {
            name: "landscape"
            PropertyChanges {
                target: grid
                flow: GridLayout.LeftToRight
                columns: 3
                rows: 3
                rowSpacing: 20
            }
            PropertyChanges {
                target: cup
                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: root.height / 1.5
                Layout.preferredWidth: root.width / 5
                Layout.column: 2
                Layout.row: 0
            }
            PropertyChanges {
                target: dialog
                Layout.preferredWidth: root.width / 4
                Layout.preferredHeight: parent.height / 4
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.column: 0
                Layout.row: 0
                Layout.columnSpan: 2
            }
            PropertyChanges {
                target: continueButton
                Layout.column: 2
                Layout.row: 2
                Layout.preferredWidth: root.width / 4
                Layout.preferredHeight: root.height / 8
            }
            PropertyChanges {
                target: cancelButton
                Layout.column: 0
                Layout.row: 2
                Layout.preferredWidth: root.width / 4
                Layout.preferredHeight: root.height / 8
            }
        }
    ]
    GridLayout {
        id: grid
        flow: GridLayout.TopToBottom
        anchors.horizontalCenter: parent.horizontalCenter
        Cup {
            id: cup
        }
        Rectangle {
            id: dialog
            radius: 8
            Layout.minimumHeight: 70
            Layout.minimumWidth: 180
            gradient: Colors.greenBorder
            Rectangle {
                id: rectangle
                width: parent.width - 2
                height: parent.height - 2
                radius: 8
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: Colors.currentTheme.cardColor
                Text {
                    text: "Please insert your cup."
                    color: Colors.currentTheme.textColor
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            MultiEffect {
                source: rectangle
                anchors.fill: rectangle
                shadowEnabled: true
                shadowColor: Colors.shadow
                shadowOpacity: 0.5
            }
        }
        CustomButton {
            id: continueButton
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.minimumWidth: 150
            Layout.minimumHeight: 40
            showIcon: false
            buttonText: "Continue"
            buttonColor: "green"
        }
        CustomButton {
            id: cancelButton
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.minimumWidth: 150
            Layout.minimumHeight: 40
            showIcon: false
            buttonText: "Cancel"
        }
    }
}
