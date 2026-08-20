// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWebSockets

Rectangle {
    width: 360
    height: 360

    function appendMessage(message) {
        messageBox.text += "\n" + message
    }

    WebSocketServer {
        id: server
        listen: true
        onClientConnected: function(webSocket) {
            webSocket.onTextMessageReceived.connect(function(message) {
                appendMessage(qsTr("Server received message: %1").arg(message));
                webSocket.sendTextMessage(qsTr("Hello Client!"));
            });
        }
        onErrorStringChanged: {
            appendMessage(qsTr("Server error: %1").arg(errorString));
        }
    }

    WebSocket {
        id: socket
        url: server.url
        onTextMessageReceived: function(message) {
            appendMessage(qsTr("Client received message: %1").arg(message));
        }
        onStatusChanged: {
            if (socket.status == WebSocket.Error) {
                appendMessage(qsTr("Client error: %1").arg(socket.errorString));
            } else if (socket.status == WebSocket.Closed) {
                appendMessage(qsTr("Client socket closed."));
            }
        }
    }

    Timer {
        interval: 100
        running: true
        onTriggered: {
            socket.active = true;
        }
    }

    Text {
        id: messageBox
        text: qsTr("Click to send a message!")
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onClicked: {
                socket.sendTextMessage(qsTr("Hello Server!"));
            }
        }
    }
}
