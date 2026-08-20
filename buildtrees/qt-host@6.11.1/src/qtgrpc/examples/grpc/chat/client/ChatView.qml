// Copyright (C) 2024 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtCore
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt.labs.qmlmodels

import QtGrpcChat
import QtGrpcChat.Proto

Item {
    id: root

    property string viewInfoText: "<b><font color='#449C47'>Qt</font></b>"
        + "<font color='#0B9895'>Grpc</font>Chat"
    readonly property real maxMessageBoxWidth: root.width * 0.75
    readonly property real margin: 7.5

    Component.onCompleted: {
        viewInfoText += " " + ChatEngine.getSchemeSymbol()
    }

    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: (dragEvent) => {
            if (!dragEvent.hasUrls) {
                console.log(qsTr("Drag event has no URLs"));
                return;
            }
            ChatEngine.sendFiles(dragEvent.urls);
        }
        onEntered: (dragEvent) => {
            inputField.forceActiveFocus()
        }
    }

    ColumnLayout {
        anchors.margins: 15
        anchors.fill: parent
        spacing: 15

//! [client-qml-4a]
        ListView {
            id: chatMessageView
//! [client-qml-4a]
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            model: ChatEngine.chatMessageModel
            spacing: 15
            onCountChanged: Qt.callLater(positionViewAtEnd)
            cacheBuffer: height * 2

            // TODO: make scrollbar smooth
            // ScrollBar.vertical: ScrollBar {}

//! [client-qml-4b]
            component DelegateBase: Item {
                id: base

                required property chatMessage display
                default property alias data: chatLayout.data
//! [client-qml-4b]

                readonly property color lightColor: Qt.lighter(box.color, 2)
                readonly property color darkColor: Qt.darker(box.color, 2)
                readonly property bool ownMessage: display.username === ChatEngine.username
                readonly property string messageOwner: ownMessage ? qsTr("You")
                    : display.username

                width: ListView.view.width
                height: childrenRect.height

                Rectangle {
                    id: box
                    anchors {
                        left: base.display.hasUserStatus || !base.ownMessage ?
                            parent.left : undefined
                        right: base.display.hasUserStatus || base.ownMessage ?
                            parent.right : undefined
                    }

                    color: base.ownMessage ? "#426340" : "#4B3F72"
                    width: chatLayout.implicitWidth
                    height: chatLayout.implicitHeight
                    radius: 5
                    clip: true

                    ColumnLayout {
                        id: chatLayout
                        spacing: root.margin

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: root.margin
                            Layout.topMargin: root.margin
                            Layout.rightMargin: root.margin
                            visible: !base.display.hasUserStatus
                            // Display message username
                            Text {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignLeft
                                font.weight: Font.Bold
                                font.pointSize: 12
                                color: base.lightColor
                                text: base.messageOwner
                            }
                            // Display message time
                            Text {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignRight
                                font.weight: Font.Bold
                                font.pointSize: 12
                                color: base.lightColor
                                text: new Date(base.display.timestamp)
                                    .toLocaleTimeString(Qt.locale(), Locale.ShortFormat)
                            }
                        }

                        // Further Components will be appended to the .data here.
                    }
                }

//! [client-qml-4c]
            }
//! [client-qml-4c]

//! [client-qml-4d]
            // We use the DelegateChooser and the 'whatThis' role to determine
            // the correct delegate for any ChatMessage
            delegate: DelegateChooser {
                role: "whatsThis"
//! [client-qml-4d]
                DelegateChoice {
                    roleValue: "continuation"
                    delegate: DelegateBase {
                        id: dbc
                        FileContinuationDelegate {
                            Layout.fillWidth: true
                            Layout.maximumWidth: root.maxMessageBoxWidth
                            Layout.preferredHeight: implicitHeight
                            Layout.bottomMargin: root.margin
                            Layout.leftMargin: root.margin
                            Layout.rightMargin: root.margin

                            message: dbc.display.file
                            textColor: dbc.lightColor
                        }
                    }
                }
                DelegateChoice {
                    roleValue: "image"
                    delegate: DelegateBase {
                        id: dbi
                        ImageDelegate {
                            Layout.fillWidth: true
                            Layout.maximumWidth: root.maxMessageBoxWidth
                            Layout.preferredHeight: heightRatio * width
                            Layout.bottomMargin: root.margin
                            Layout.leftMargin: root.margin
                            Layout.rightMargin: root.margin

                            message: dbi.display.file
                        }
                    }
                }
                DelegateChoice {
                    roleValue: "file"
                    delegate: DelegateBase {
                        id: dbf
                        FileDelegate {
                            Layout.fillWidth: true
                            Layout.maximumWidth: root.maxMessageBoxWidth
                            Layout.preferredHeight: implicitHeight
                            Layout.bottomMargin: root.margin
                            Layout.leftMargin: root.margin
                            Layout.rightMargin: root.margin

                            base: dbf
                            maxMessageBoxWidth: root.maxMessageBoxWidth
                        }
                    }
                }
//! [client-qml-4e]
                DelegateChoice {
                    roleValue: "text"
                    delegate: DelegateBase {
                        id: dbt
                        TextDelegate {
                            Layout.fillWidth: true
                            Layout.maximumWidth: root.maxMessageBoxWidth
                            Layout.preferredHeight: implicitHeight
                            Layout.bottomMargin: root.margin
                            Layout.leftMargin: root.margin
                            Layout.rightMargin: root.margin

                            message: dbt.display.text
                            selectionColor: dbt.lightColor
                            selectedTextColor: dbt.darkColor
                        }
                    }
                }
//! [client-qml-4e]
                DelegateChoice {
                    roleValue: "userStatus"
                    delegate: DelegateBase {
                        id: dbu
                        UserStatusDelegate {
                            Layout.fillWidth: true
                            Layout.preferredHeight: implicitHeight
                            Layout.topMargin: root.margin
                            Layout.bottomMargin: root.margin
                            Layout.leftMargin: (dbu.width / 2) - (contentWidth / 2)
                            Layout.rightMargin: root.margin

                            base: dbu
                        }
                    }
                }
            }
        }

        // Text Input & Send Button
        RowLayout {
            spacing: 10
            Flickable {
                Layout.preferredHeight: inputField.implicitHeight
                Layout.maximumHeight: root.height / 2
                Layout.fillWidth: true

                ScrollBar.horizontal: ScrollBar {}
                ScrollBar.vertical: ScrollBar {}

                boundsBehavior: Flickable.StopAtBounds
                clip: true

//! [client-qml-6a]
                TextArea.flickable: TextArea {
                    id: inputField

                    function sendTextMessage() : void {
                        if (text === "")
                            return
                        ChatEngine.sendText(text)
                        text = ""
                    }
//! [client-qml-6a]

                    padding: 10
                    antialiasing: true
                    color: "#cecfd5"
                    placeholderTextColor: "#9d9faa"
                    placeholderText: dropArea.containsDrag
                        ? qsTr("Drop any file here")
                        : qsTr("What's on your mind <b><i>") + ChatEngine.username + "</i></b> ?"
                    // Workaround for QTBUG{92371,68583,65718} on android.
                    inputMethodHints: Qt.ImhNoPredictiveText
//! [client-qml-6b]
                    Keys.onPressed: (event) => {
                        if (event.key === Qt.Key_Return && event.modifiers & Qt.ControlModifier) {
                            sendTextMessage()
                            event.accepted = true
                        } else if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                            if (ChatEngine.sendFilesFromClipboard())
                                event.accepted = true
                        }
                    }
//! [client-qml-6b]
                }
            }

            Button {
                readonly property bool sendState: inputField.text !== ""

                Layout.preferredHeight: 60
                Layout.preferredWidth: 50
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                Layout.bottomMargin: 0
                icon.source: sendState ? "res/send.svg" : "res/attach.svg"
                icon.width: 50
                icon.height: 50
                onPressed: sendState ? inputField.sendTextMessage() : fileDialog.open()
                enabled: !sendState || inputField.text
            }
        }
    }

    FileDialog {
        id: fileDialog
        options: FileDialog.ReadOnly
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        onAccepted: ChatEngine.sendFiles(selectedFiles);
    }

}

