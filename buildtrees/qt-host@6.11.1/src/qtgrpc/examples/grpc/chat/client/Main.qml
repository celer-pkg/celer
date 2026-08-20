// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

//! [client-qml-1]
import QtGrpc
import QtGrpcChat
import QtGrpcChat.Proto
//! [client-qml-1]

ApplicationWindow {
    id: root

    readonly property color darkColor: Qt.darker(color, 1.4)
    readonly property color lightColor: Qt.lighter(color, 1.4)

    Material.theme: Material.Dark
    Material.accent: Material.Purple

    width: 460
    height: 740
    title: qsTr("QtGrpcChat-Client")
    visible: true

    function handleStatus(status) : void {
        switch (status.code) {
            case QtGrpc.StatusCode.Ok:
                statusDisplay.text = status.message === "" ? qsTr("OK") : status.message
                statusDisplay.color = "#4CAF50"
                break
            case QtGrpc.StatusCode.Unavailable:
                statusDisplay.text = qsTr("Server Unavailable")
                statusDisplay.color = "#F44336"
                break
            default:
                statusDisplay.text = status.message
                statusDisplay.color = "#F44336"
                break
        }
        statusDisplay.restart()
    }

//! [client-qml-2]
    Connections {
        target: ChatEngine
        function onChatStreamFinished(status) {
            root.handleStatus(status)
            loginView.clear()
        }
        function onChatStateChanged() {
            if (ChatEngine.chatState === Backend.ChatState.Connected && mainView.depth === 1)
                mainView.push("ChatView.qml")
            else if (ChatEngine.chatState === Backend.ChatState.Disconnected && mainView.depth > 1)
                mainView.pop()
        }
        function onRegisterFinished(status) {
            root.handleStatus(status)
        }
        function onChatError(message) {
            statusDisplay.text = message
            statusDisplay.color = "yellow"
            statusDisplay.restart()
        }
    }
//! [client-qml-2]

    // Navigation header containing buttons and information text
    header: Rectangle {
        height: 40 + root.SafeArea.margins.top
        color: Qt.darker(root.darkColor, 1.2)
        RowLayout {
            anchors.fill: parent
            anchors.topMargin: root.SafeArea.margins.top
            anchors.leftMargin: 15
            anchors.rightMargin: 15

            ToolButton {
                id: settingsButton
                Layout.alignment: Qt.AlignLeft
                Layout.fillHeight: true
                Layout.preferredWidth: userStatusButton.implicitWidth
                Material.roundedScale: Material.SmallScale
                icon.source: "res/gear.svg"
                onPressed: settingsMenu.popup(settingsButton, 0, height)

                Menu {
                    id: settingsMenu

                    MenuItem {
                        text: "Show Logs"
                        onTriggered: logsDialog.open()
                    }

                    MenuSeparator {
                        visible: mainView.depth > 1
                        height: visible ? implicitHeight : 0
                    }

                    MenuItem {
                        text: "Logout"
                        visible: mainView.depth > 1
                        height: visible ? implicitHeight : 0
                        onTriggered: {
                            if (userStatusButton.checked)
                                userStatusButton.animateClick()
                            ChatEngine.logout()
                        }
                    }
                }
            }

            Text {
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: Material.foreground
                text: mainView.currentItem.viewInfoText
                font.pointSize: 16
                clip: true
            }

            Button {
                id: userStatusButton

                Layout.alignment: Qt.AlignRight
                Layout.fillHeight: true
                Layout.preferredWidth: 120
                Material.roundedScale: Material.SmallScale

                enabled: mainView.depth > 1
                clip: true
                checkable: true
                text: userStatusView.count + " ONLINE"
                onPressed: userStatus.height = checked ? 0 : userStatusView.contentHeight
            }
        }
    }

    // Visualize online users
    Item {
        id: userStatus
        width: parent.width * 0.75
        anchors.horizontalCenter: parent.horizontalCenter
        height: 0
        z: 1
        Behavior on height {
            NumberAnimation { duration: 250 }
        }
        ListView {
            id: userStatusView
            anchors.fill: parent
            model: ChatEngine.userStatusModel
            interactive: false
            clip: true
            delegate: Rectangle {
                id: userStatusDelegate

                required property int index
                required property string username
                required property int status

                height: 32
                width: userStatusView.width
                color: root.lightColor
                topLeftRadius: index === 0 ? 8 : 0
                topRightRadius: index === 0 ? 8 : 0
                bottomLeftRadius: index === userStatusView.count - 1 ? 8 : 0
                bottomRightRadius: index === userStatusView.count - 1 ? 8 : 0

                Rectangle {
                    height: 2
                    width: parent.width
                    anchors.top: parent.top
                    color: Qt.lighter(parent.color, 1.4)
                    visible: userStatusDelegate.index > 0
                }

                Text {
                    text: parent.username
                    font.pointSize: 14
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: Material.foreground
                }
                Rectangle {
                    anchors.right: parent.right
                    anchors.margins: 25
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height - 16
                    width: height
                    radius: height / 2
                    color: {
                        switch (parent.status) {
                            case UserStatus.Type.LOGIN:
                                return "#4CAF50"
                            case UserStatus.Type.ACTIVE:
                                return "#4CAF50"
                            case UserStatus.Type.INACTIVE:
                                return "#FF9800"
                            case UserStatus.Type.LOGOUT:
                                return "#F44336"
                        }
                    }
                }
            }
        }
    }

    StatusDisplay {
        id: statusDisplay
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            leftMargin: 75
            rightMargin: 75
            topMargin: root.height / 4
        }
    }

    BusyIndicator {
        id: busyIndicator
        z: 2
        width: parent.width / 3
        height: width
        anchors.centerIn: parent
        running: ChatEngine.chatState === Backend.ChatState.Connecting
            || ChatEngine.chatState === Backend.ChatState.Disconnecting
    }

    StackView {
        id: mainView
        anchors.fill: parent
        initialItem: Item {

            readonly property string viewInfoText: "Login"

            LoginView {
               id: loginView
               anchors {
                   fill: parent
                   leftMargin: 50
                   rightMargin: 50
                   topMargin: 100
                   bottomMargin: 100
               }
               color: root.darkColor
               border.color: root.lightColor
            }
        }
    }
    LogDialog {
        id: logsDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: root.width * 0.95
        height: root.height * 0.8
        leftPadding: 4
        rightPadding: 4
    }
}
