// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2016 basysKom GmbH, author Bernd Lamecker <bernd.lamecker@basyskom.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts
import QtWebSockets
import "qrc:/qwebchannel.js" as WebChannel

ApplicationWindow {
    id: root

    property var channel
    property string loginName: loginUi.userName.text
    property bool loggedIn: false

    title: "Chat client"
    width: 640
    height: 480
    visible: true

    WebSocket {
        id: socket

        // the following three properties/functions are required to align the QML WebSocket API
        // with the HTML5 WebSocket API.
        property var send: function(arg) {
            sendTextMessage(arg);
        }

        onTextMessageReceived: function(message) {
            onmessage({data: message});
        }

        property var onmessage

        active: true
        url: "ws://localhost:12345"

        onStatusChanged: {
            switch (socket.status) {
            case WebSocket.Error:
                errorDialog.text = "Error: " + socket.errorString;
                errorDialog.visible = true;
                break;
            case WebSocket.Closed:
                errorDialog.text = "Error: Socket at " + url + " closed.";
                errorDialog.visible = true;
                break;
            case WebSocket.Open:
                //open the webchannel with the socket as transport
                new WebChannel.QWebChannel(socket, function(ch) {
                    root.channel = ch;

                    //connect to the changed signal of the userList property
                    ch.objects.chatserver.userListChanged.connect(function(args) {
                        mainUi.userlist.text = '';
                        ch.objects.chatserver.userList.forEach(function(user) {
                            mainUi.userlist.text += user + '\n';
                        });
                    });

                    //connect to the newMessage signal
                    ch.objects.chatserver.newMessage.connect(function(time, user, message) {
                        var line = "[" + time + "] " + user + ": " + message + '\n';
                        mainUi.chat.text = mainUi.chat.text + line;
                    });

                    //connect to the keep alive signal
                    ch.objects.chatserver.keepAlive.connect(function(args) {
                        if (loginName !== '' && root.loggedIn)
                            //and call the keep alive response method as an answer
                            ch.objects.chatserver.keepAliveResponse(loginName);
                    });
                });

                loginWindow.show();
                break;
            }
        }
    }

    MainForm {
        id: mainUi
        anchors.fill: parent

        Connections {
            target: mainUi.message
            function onEditingFinished() {
                if (mainUi.message.text.length) {
                    //call the sendMessage method to send the message
                    root.channel.objects.chatserver.sendMessage(loginName,
                                                                mainUi.message.text);
                }
                mainUi.message.text = '';
            }
        }
    }

    ApplicationWindow {
        id: loginWindow

        title: "Login"
        modality: Qt.ApplicationModal
        flags: Qt.CustomizeWindowHint | Qt.WindowTitleHint
        width: 300
        height: 200

        LoginForm {
            id: loginUi
            anchors.fill: parent

            nameInUseError.visible: false

            Connections {
                target: loginUi.loginButton

                function onClicked() {
                    if (loginName === '')
                        return;
                    //call the login method
                    root.channel.objects.chatserver.login(loginName, function(arg) {
                        //check the return value for success
                        if (arg === true) {
                            loginUi.nameInUseError.visible = false;
                            loginWindow.close();
                            root.loggedIn = true;
                        } else {
                            loginUi.nameInUseError.visible = true;
                        }
                    });
                }
            }
        }
    }

    Dialog {
        id: errorDialog
        property alias text: message.text

        anchors.centerIn: parent
        standardButtons: Dialog.Close
        title: "Chat client"
        width: parent.width / 2

        Label {
            id: message
        }

        onAccepted: {
            Qt.quit();
        }
        onRejected: {
            Qt.quit();
        }
    }
}
