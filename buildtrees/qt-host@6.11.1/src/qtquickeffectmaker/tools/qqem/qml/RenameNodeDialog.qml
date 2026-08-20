// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

CustomDialog {
    id: rootItem
    title: qsTr("Rename Node")
    width: 400
    height: 340
    modal: true
    focus: true
    function checkNameOK() {
        // Name must not be empty
        var name = nameTextItem.text;
        var button = rootItem.standardButton(Dialog.Ok);
        if (name.length < 1) {
            button.enabled = false;
        } else {
            button.enabled = true;
        }
    }

    CustomTextField {
        id: nameTextItem
        width: parent.width
        text: effectManager.nodeView.selectedNodeName
        onTextChanged: {
            checkNameOK();
        }
    }
    CustomTextEdit {
        id: descriptionTextItem
        anchors.top: nameTextItem.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        width: parent.width
        text: effectManager.nodeView.selectedNodeDescription
        Text {
            anchors.centerIn: parent
            text: "(Description)"
            visible: descriptionTextItem.text === ""
            color: mainView.foregroundColor1
            font.pixelSize: 16
        }
    }

    standardButtons: Dialog.Ok | Dialog.Cancel
    onAccepted: {
        effectManager.nodeView.selectedNodeName = nameTextItem.text;
        effectManager.nodeView.selectedNodeDescription = descriptionTextItem.text;
    }
}
