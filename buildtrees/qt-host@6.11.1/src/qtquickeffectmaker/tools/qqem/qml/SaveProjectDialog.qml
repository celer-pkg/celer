// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

CustomDialog {
    id: root

    // Which action user is trying to do when this dialog gets shown
    // 0: New Project
    // 1: Open Project
    // 2: Close Project
    // 3: Close Application
    property int action: -1

    // File user is trying to open when this dialog gets shown
    property string openFileName: ""

    title: qsTr("Save Project Changes")
    width: 320
    height: 200
    modal: true
    focus: true
    standardButtons: Dialog.Save | Dialog.Discard | Dialog.Cancel
    closePolicy: Popup.NoAutoClose

    Text {
        width: parent.width
        text: "Project '" + effectManager.projectName + "' contains unsaved changes."
        font.pixelSize: 14
        color: mainView.foregroundColor2
        wrapMode: Text.WordWrap
    }
}
