// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Dialogs
import QtCore

CustomDialog {
    id: root

    property string defaultPath: StandardPaths.standardLocations(StandardPaths.DesktopLocation)[0]
    property bool clearNodeView: false
    property bool exportNext: false

    title: qsTr("New Effect Project")
    width: 540
    height: 400
    modal: true
    focus: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    closePolicy: Popup.NoAutoClose

    Component.onCompleted: {
        pathTextEdit.text = effectManager.stripFileFromURL(defaultPath);
        nameTextEdit.text = "Effect01"
    }

    FolderDialog {
        id: projectPathDialog
        currentFolder: defaultPath
        onAccepted: {
            if (currentFolder) {
                // Remove file start
                var usedFolder = effectManager.stripFileFromURL(currentFolder.toString());
                pathTextEdit.text = usedFolder;
            }
        }
    }

    GridLayout {
        width: parent.width
        columns: 3
        columnSpacing: 20
        Label {
            text: qsTr("Name:")
            font.bold: true
            font.pixelSize: 14
            color: mainView.foregroundColor2
        }
        CustomTextField {
            id: nameTextEdit
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: effectManager.projectName
        }
        Label {
            text: qsTr("Create in:")
            font.bold: true
            font.pixelSize: 14
            color: mainView.foregroundColor2
        }
        CustomTextField {
            id: pathTextEdit
            Layout.fillWidth: true
            text: effectManager.projectFilename
        }
        Button {
            Layout.preferredHeight: 40
            text: qsTr("Browse");
            onClicked: {
                projectPathDialog.open();
            }
        }
    }
    onAccepted: {
        mainView.mainToolbar.setDesignModeInstantly(true);
        effectManager.newProject(pathTextEdit.text, nameTextEdit.text, clearNodeView, true);
        if (root.exportNext) {
            // Proceed next with exporting
            mainWindow.exportAction();
            root.exportNext = false;
        }
    }
}
