// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Item {
    id: home
    property alias getStartedbutton: getStartedButton
    property alias grid: grid
    property alias header: header
    property alias caption: caption

    states: [
        State {
            name: "portrait"
            PropertyChanges {
                target: getStartedButton
                Layout.preferredWidth: home.width / 2.2
                Layout.preferredHeight: home.height / 14
                Layout.topMargin: 20
            }
        },
        State {
            name: "landscape"
            PropertyChanges {
                target: grid
                anchors.topMargin: 0
            }
            PropertyChanges {
                target: header
                Layout.topMargin: 0
            }
            PropertyChanges {
                target: getStartedButton
                Layout.preferredWidth: home.width / 4
                Layout.preferredHeight: home.height / 8
                Layout.topMargin: 0
            }
            PropertyChanges {
                target: getStartedButton
                Layout.minimumHeight: 0
            }
        }
    ]

    GridLayout {
        id: grid
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: home.top
        flow: GridLayout.TopToBottom
        Image {
            id: image
            Layout.preferredHeight: home.height / 3
            Layout.preferredWidth: height / 1.16
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.margins: 5
            source: (Colors.currentTheme == Colors.dark) ? "./images/Cups/home_dark.svg" : "./images/Cups/home_light.svg"
        }
        Text {
            id: header
            text: qsTr("Coffee Machine")
            font.pixelSize: 32
            font.weight: 700
            color: Colors.currentTheme.textColor
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.topMargin: 20
        }
        Text {
            id: caption
            text: qsTr(" pick your blend, tailor your flavors, and savor the \n perfection!")
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            Layout.maximumWidth: home.width
            wrapMode: Text.Wrap
            font.weight: 400
            color: Colors.currentTheme.caption
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        }
        CustomButton {
            id: getStartedButton
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.minimumWidth: 150
            Layout.minimumHeight: 40
            buttonText: "Get Started"
        }
    }
}
