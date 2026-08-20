// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import chat_example

pragma ComponentBehavior: Bound

Item {
    id: root
    anchors.fill: parent

    signal goBack();

    onGoBack: {
        conversationModel.clear()
        messageField.clear()
        OpenAIChatManager.customUserCharacter = ""
    }

    //! [0]
    function editingFinished() {
        var messageObject = {"name": "Me", "message": messageField.text };
        conversationModel.append(messageObject);
        OpenAIChatManager.userRequest = messageField.text;
        OpenAIChatManager.sendUserRequest();
        messageField.text = "";
    }
    //! [0]

    ListModel {
        id: conversationModel
    }

    Connections {
        target: OpenAIChatManager
        function onResponseReady(response) {
            var responseMessage = {"name": OpenAIChatManager.modelName, "message": response };
            conversationModel.append(responseMessage)
            listView.positionViewAtEnd()
        }
    }

    Item {
        id: chatBgBorder
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.93
        height: parent.height * 0.70

        Pane {
            id: chatPane
            anchors.fill: chatBgBorder
            anchors.margins: 10

            background: Rectangle {
                radius: 10
                color: Material.backgroundColor
            }

            ListView {
                id: listView
                anchors.fill: parent

                verticalLayoutDirection: ListView.TopToBottom
                spacing: 12

                clip: true
                model: conversationModel

                delegate: Column {
                    id: columnElement

                    anchors.right: sentByMe ? listView.contentItem.right : undefined
                    anchors.left: !sentByMe ? listView.contentItem.left : undefined

                    spacing: 6
                    required property string name
                    required property string message
                    readonly property bool sentByMe: name === "Me"
                    Row {
                        id: messageRow
                        spacing: 6
                        anchors.right: columnElement.sentByMe ? parent.right : undefined
                        anchors.rightMargin: 30
                        anchors.left: !columnElement.sentByMe ? parent.left : undefined
                        anchors.leftMargin: 10

                        Rectangle {
                            width: Math.min(messageText.implicitWidth + 24,
                                            listView.width - messageRow.spacing - 30)
                            height: messageText.implicitHeight + 24
                            color: "#1E1E1E"
                            radius: 10

                            gradient: Gradient {
                                GradientStop { position: 0.0; color: columnElement.sentByMe
                                                                     ? "#4A148C" : "#1C2B3A" }
                                GradientStop { position: 1.0; color: columnElement.sentByMe
                                                                     ? "#7B1FA2" : "#16222E" }
                            }

                            Behavior on gradient {
                                NumberAnimation { duration: 150 }
                            }

                            Label {
                                id: messageText

                                anchors.fill: parent
                                anchors.margins: 12

                                text: columnElement.message
                                font.pointSize: 16
                                wrapMode: Label.Wrap
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    id: messagesScroll

                    width: 15
                    policy: ScrollBar.AsNeeded

                    contentItem: Rectangle {
                        radius: 10
                        color: messagesScroll.pressed ? "#1C2B3A" : "#16222E"
                    }

                    background: Rectangle {
                        color: "#2C2C2C"
                        radius: 9
                    }
                }
            }
        }
    }

    Item {
        id: editPanel
        anchors.top: chatBgBorder.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: root.horizontalCenter

        width: parent.width * 0.93 - 20
        height: parent.height * 0.25 - backButton.height

        ScrollView {
            id: textBg
            width: editPanel.width
            height: editPanel.height

            anchors.centerIn: parent
            anchors.margins: 10

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

            TextArea {
                id: messageField

                font.pointSize: 16
                placeholderText: qsTr("Enter your question")
                wrapMode: TextArea.Wrap
                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        if (event.modifiers === Qt.NoModifier) {
                            event.accepted = true
                            sendButton.click()
                        } else if (event.modifiers & Qt.ShiftModifier) {
                            event.accepted = false
                        }
                    }
                }
            }
        }
    }

    Row {
        id: buttonLayout
        anchors.top: editPanel.bottom
        anchors.horizontalCenter : parent.horizontalCenter
        anchors.margins: 10
        spacing: 100

        Button {
            id: backButton

            text: qsTr("Back")
            font.pointSize: 16

            onClicked: {
                root.goBack()
            }
        }

        Button {
            id: sendButton

            text: qsTr("Send")
            font.pointSize: 16

            enabled: messageField.length > 0
            onClicked: {
                root.editingFinished();
                listView.positionViewAtEnd()
            }
        }
    }
}
