// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import chat_example

Rectangle {
    id: root

    anchors.centerIn: parent

    color: Material.backgroundColor

    signal closePage()
    signal openChatPage()

    ScrollView {
        id: textBgScroll

        width: parent.width * 0.9
        height: parent.height * 0.8

        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        background: Rectangle {
            color: "#1E1E1E"
            radius: 10

            gradient: Gradient {
                GradientStop { position: 0.0; color: "#4A148C" }
                GradientStop { position: 1.0; color: "#7B1FA2" }
            }

            Behavior on gradient {
                NumberAnimation { duration: 150 }
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: vbar

            anchors.top: parent.top
            anchors.topMargin: 19
            anchors.right: parent.right
            anchors.rightMargin: 13
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 7
            width: 15
            height: parent.height

            policy: ScrollBar.AsNeeded
            contentItem: Rectangle {
                radius: 10
                color: vbar.pressed ? "#4DA8C7E6" : "#4D8FB3D9"
            }

            background: Rectangle {
                color: "#33FFFFFF"
                radius: 9
            }
        }

        TextArea {
            id: messageField

            font.pointSize: 14
            placeholderText: qsTr("Describe a person you want to talk to")
            wrapMode: TextArea.Wrap
        }
    }

    Button {
        id: cancelButton

        anchors.left: textBgScroll.left
        anchors.top: textBgScroll.bottom
        anchors.topMargin: 10

        text: qsTr("Cancel")
        font.pointSize: 16

        onClicked: {
            messageField.text = ""
            root.closePage()
        }
    }

    Button {
        id: sendButton

        anchors.right: textBgScroll.right
        anchors.top: textBgScroll.bottom
        anchors.topMargin: 10

        width: cancelButton.width
        height: cancelButton.height
        text: qsTr("Send")
        font.pointSize: 16
        enabled: messageField.length > 0

        onClicked: {
            OpenAIChatManager.customUserCharacter = messageField.text
            OpenAIChatManager.characterId = OpenAIChatManager.UserCharacter
            messageField.text = ""
            root.openChatPage()
        }
    }
}
