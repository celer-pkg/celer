// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import NdefEditorModule

ApplicationWindow {
    id: window

    enum TargetDetectedAction {
        NoAction,
        ReadMessage,
        WriteMessage
    }

    StackView {
        id: stack
        initialItem: mainView
        anchors.fill: parent
    }

    property int targetDetectedAction: MainWindow.NoAction

    Component {
        id: mainView

        ColumnLayout {
            ToolBar {
                Layout.fillWidth: true

                RowLayout {
                    anchors.fill: parent
                    Label {
                        text: qsTr("NDEF Editor")
                        elide: Label.ElideRight
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                    }
                }
            }

            Pane {
                id: writeTab
                Layout.fillWidth: true
                Layout.fillHeight: true

                Flickable {
                    anchors.fill: parent
                    contentHeight: contentItem.childrenRect.height

                    ColumnLayout {
                        width: parent.width

                        ItemDelegate {
                            icon.name: "file_download"
                            text: qsTr("Read Tag")
                            Layout.fillWidth: true

                            onClicked: window.readTag()
                        }

                        ItemDelegate {
                            icon.name: "file_upload"
                            text: qsTr("Write to Tag")
                            Layout.fillWidth: true

                            onClicked: window.writeTag()
                        }

                        ItemDelegate {
                            icon.name: "add"
                            text: qsTr("Add a Record")
                            Layout.fillWidth: true

                            onClicked: addRecordMenu.open()

                            Menu {
                                id: addRecordMenu
                                title: qsTr("Add record")

                                MenuItem {
                                    icon.name: "text_snippet"
                                    text: qsTr("Text record")
                                    onTriggered: stack.push(textRecordDialog)
                                }
                                MenuItem {
                                    icon.name: "link"
                                    text: qsTr("URI record")
                                    onTriggered: stack.push(uriRecordDialog)
                                }
                            }
                        }

                        Item {
                            implicitHeight: 20
                        }

                        Label {
                            text: qsTr(
                                "Use buttons above to read an NFC tag or add records manually.\n\n"
                                + "Once added, swipe to the right to access options for each record.")
                            wrapMode: Text.Wrap
                            visible: messages.count === 0
                            Layout.fillWidth: true
                        }

                        Repeater {
                            id: messages
                            model: messageModel

                            delegate: NdefRecordDelegate {
                                Layout.fillWidth: true

                                onDeleteClicked: messageModel.removeRow(index)

                                onClicked: {
                                    if (recordType === NdefMessageModel.TextRecord) {
                                        stack.push(textRecordDialog, {
                                            text: recordText,
                                            modelIndex: index
                                        })
                                    } else if (recordType === NdefMessageModel.UriRecord) {
                                        stack.push(uriRecordDialog, {
                                            uri: recordText,
                                            modelIndex: index
                                        })
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    NdefMessageModel {
        id: messageModel
    }

    NfcManager {
        id: nfcManager
        onTargetDetected: (target) => { window.handleTargetDetected(target) }
    }

    Connections {
        id: targetConnections
        target: null

        function onNdefMessageRead(message) {
            messageModel.message = message
        }

        function onRequestCompleted() {
            communicationOverlay.close()
            window.targetDetectedAction = MainWindow.NoAction
            releaseTarget()
        }

        function onError(code) {
            communicationOverlay.close()
            errorNotification.errorMessage = window.describeError(code)
            errorNotification.open()
            window.targetDetectedAction = MainWindow.NoAction
            releaseTarget()
        }

        function releaseTarget() {
            target?.destroy()
        }
    }

    function handleTargetDetected(target: NfcTarget) {
        targetConnections.releaseTarget()

        let ok = true

        if (targetDetectedAction === MainWindow.ReadMessage) {
            targetConnections.target = target
            communicationOverlay.message = qsTr("Target detected. Reading messages...")
            errorNotification.errorMessage = qsTr("NDEF read error")
            ok = target.readNdefMessages()
        } else if (targetDetectedAction === MainWindow.WriteMessage) {
            targetConnections.target = target
            communicationOverlay.message = qsTr("Target detected. Writing the message...")
            errorNotification.errorMessage = qsTr("NDEF write error")
            ok = target.writeNdefMessage(messageModel.message)
        } else {
            target.destroy()
            return
        }

        if (!ok) {
            communicationOverlay.close()
            errorNotification.open()
            targetConnections.releaseTarget()
        }
    }

    function readTag() {
        window.targetDetectedAction = MainWindow.ReadMessage
        nfcManager.startTargetDetection()
        communicationOverlay.title = qsTr("Read Tag")
        communicationOverlay.message = qsTr("Approach an NFC tag.")
        communicationOverlay.open()
    }

    function writeTag() {
        window.targetDetectedAction = MainWindow.WriteMessage
        nfcManager.startTargetDetection()
        communicationOverlay.title = qsTr("Write to Tag")
        communicationOverlay.message = qsTr("Approach an NFC tag.")
        communicationOverlay.open()
    }

    function describeError(error) {
        switch (error) {
        case 2: return qsTr("Usupported feature")
        case 3: return qsTr("Target out of range")
        case 4: return qsTr("No response")
        case 5: return qsTr("Checksum mismatch")
        case 6: return qsTr("Invalid parameters")
        case 7: return qsTr("Connection error")
        case 8: return qsTr("NDEF read error")
        case 9: return qsTr("NDEF write error")
        case 10: return qsTr("Command error")
        case 11: return qsTr("Timeout")
        }

        return qsTr("Unknown error")
    }

    Dialog {
        id: communicationOverlay

        property alias message: messageLabel.text

        anchors.centerIn: Overlay.overlay

        modal: true
        standardButtons: Dialog.Cancel

        ColumnLayout {
            Label {
                id: messageLabel
            }

            BusyIndicator {
                Layout.fillWidth: true
            }
        }

        onClosed: nfcManager.stopTargetDetection()
    }

    MessageDialog {
        property alias errorMessage: errorNotification.text

        id: errorNotification

        buttons: MessageDialog.Close
        title: qsTr("Error")

        onButtonClicked: errorNotification.close()
    }

    Component {
        id: uriRecordDialog

        ColumnLayout {
            property alias uri: uriEditor.text
            property int modelIndex: -1

            ToolBar {
                Layout.fillWidth: true

                RowLayout {
                    anchors.fill: parent
                    ToolButton {
                        icon.name: "arrow_back"
                        onClicked: stack.pop()
                    }
                    Label {
                        text: qsTr("URI Record")
                        elide: Label.ElideRight
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        text: qsTr("Save")
                        onClicked: {
                            if (modelIndex < 0) {
                                messageModel.addUriRecord(uri)
                            } else {
                                messageModel.setTextData(modelIndex, uri)
                            }
                            stack.pop()
                        }
                    }
                }
            }

            Pane {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent

                    TextField {
                        id: uriEditor
                        Layout.fillWidth: true
                        placeholderText: qsTr("https://qt.io")
                        inputMethodHints: Qt.ImhUrlCharactersOnly
                    }
                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }

    Component {
        id: textRecordDialog

        ColumnLayout {
            property alias text: textEditor.text
            property int modelIndex: -1

            ToolBar {
                Layout.fillWidth: true

                RowLayout {
                    anchors.fill: parent
                    ToolButton {
                        icon.name: "arrow_back"
                        onClicked: stack.pop()
                    }
                    Label {
                        text: qsTr("Text Record")
                        elide: Label.ElideRight
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        text: qsTr("Save")
                        onClicked: {
                            if (modelIndex < 0) {
                                messageModel.addTextRecord(textEditor.text)
                            } else {
                                messageModel.setTextData(modelIndex, textEditor.text)
                            }
                            stack.pop()
                        }
                    }
                }
            }

            Pane {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ScrollView {
                    anchors.fill: parent

                    TextArea {
                        id: textEditor
                        placeholderText: qsTr("Enter some text...")
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
