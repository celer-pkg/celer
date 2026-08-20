// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtCore

import QtGrpcChat
import QtGrpcChat.Proto

//! [client-qml-3a]
Rectangle {
    id: root

    property credentials creds
//! [client-qml-3a]

    function clear() : void {
        creds.name = ""
        creds.password = ""
        nameField.text = ""
        passwordField.text = ""
    }

    radius: 25
    border.width: 4

//! [client-qml-3b]
    ColumnLayout {
        id: credentialsItem
//! [client-qml-3b]
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        clip: true

        Item { Layout.fillHeight: true }

        TextField {
            id: nameField

            Layout.fillWidth: true
            Layout.preferredHeight: 50

            placeholderText: qsTr("user name")
            focus: true
        }
        TextField {
            id: passwordField

            Layout.fillWidth: true
            Layout.preferredHeight: 50

            placeholderText: qsTr("user password")
            inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData
            echoMode: TextInput.Password
            font.pointSize: 14
        }

//! [client-qml-3c]
        RowLayout {
            id: buttonLayout
//! [client-qml-3c]
            Layout.preferredHeight: 60
            spacing: 10

//! [client-qml-3d]
            Button {
                id: loginButton
//! [client-qml-3d]

                Material.roundedScale: Material.SmallScale
                Layout.fillWidth: true
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.tab: registerButton

//! [client-qml-3e]
                enabled: nameField.text && passwordField.text
                text: qsTr("Login")
                onPressed: {
                    root.creds.name = nameField.text
                    root.creds.password = passwordField.text
                    ChatEngine.login(root.creds)
                }
            }
//! [client-qml-3e]
            Button {
                id: registerButton

                Material.roundedScale: Material.SmallScale
                Layout.fillWidth: true
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.tab: nameField

                enabled: nameField.text && passwordField.text
                text: qsTr("Register")
                onPressed: {
                    root.creds.name = nameField.text
                    root.creds.password = passwordField.text
                    ChatEngine.registerUser(root.creds)
                }
            }
        }

        Item { Layout.fillHeight: true }

        Button {
            Material.roundedScale: Material.SmallScale
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: parent.width / 2
            text: qsTr("Settings")
            onPressed: settingsDialog.open()
        }

    }

    Dialog {
        id: settingsDialog

        title: qsTr("Connection Settings")
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        clip: true

        onOpened: {
            const ips = ChatEngine.findLocalIps();
            if (ips.length === 0)
                clientIpField.text = qsTr("No network connection");
            else
                clientIpField.text = ips.join(", ");
            if (settings.hostUri !== "")
                ChatEngine.hostUri = settings.hostUri
            hostUriField.text = ChatEngine.hostUri
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Item { Layout.fillHeight: true }

            Text {
                text: qsTr("Detected IPs for This Machine:")
                color: "gray"
                font.bold: true
            }
            Text {
                id: clientIpField
                Layout.leftMargin: 20
                Layout.bottomMargin: 20
                Layout.maximumWidth: parent.width - 40
                color: "gray"
                font.italic: true
                wrapMode: TextEdit.Wrap
            }

            Text {
                text: qsTr("Available Server ports: ")
                color: "gray"
                font.bold: true
            }
            Repeater {
                model: ChatEngine.displayServerPorts()
                Text {
                    required property string modelData
                    Layout.leftMargin: 20
                    text: modelData
                    color: "gray"
                    font.italic: true
                }
            }

            Text {
                text: qsTr("Current Server URI: ")
                color: "gray"
                font.bold: true
            }
            Text {
                Layout.leftMargin: 20
                Layout.bottomMargin: 20
                text: ChatEngine.hostUri
                color: "gray"
                font.italic: true
            }

            TextField {
                id: hostUriField
                Layout.fillWidth: true
                placeholderText: qsTr("New Server URI")
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.topMargin: 20
                Layout.alignment: Qt.AlignRight
                spacing: 10

                Button {
                    text: qsTr("Cancel")
                    onClicked: settingsDialog.reject()
                }

                Button {
                    text: qsTr("OK")
                    onClicked: {
                        if (hostUriField.text === "") {
                            settingsDialog.reject()
                            return;
                        }
                        ChatEngine.hostUri = hostUriField.text
                        settings.hostUri = hostUriField.text
                        settingsDialog.accept()
                    }
                }
            }
        }

    }

    Settings {
        id: settings
        property string hostUri: ""
    }
}
