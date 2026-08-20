// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs

ApplicationWindow {
    id: mainWindow

    property alias effectManager: mainView.effectManager
    readonly property string isUnsaved: effectManager.unsavedChanges ? "*" : ""
    readonly property string projectTitle: effectManager.projectName ? (isUnsaved + effectManager.projectName + " - ") : ""
    property alias propertyEditDialog: mainView.propertyEditDialog
    property alias currentEditorComponent: mainView.currentEditorComponent
    property bool canCut: !mainView.designMode
    property bool canCopy: !mainView.designMode
    property bool canPaste: !mainView.designMode && currentEditorComponent ? currentEditorComponent.canPaste : false
    property bool canUndo: !mainView.designMode && currentEditorComponent ? currentEditorComponent.canUndo : false
    property bool canRedo: !mainView.designMode && currentEditorComponent ? currentEditorComponent.canRedo : false
    property bool canFind: !mainView.designMode && currentEditorComponent
    property bool quitAccepted: false

    onClosing: function(close) {
        maybeQuitAction();
        close.accepted = mainWindow.quitAccepted;
    }

    width: 1200
    height: 800
    minimumWidth: 600
    minimumHeight: 400
    visible: true
    title: qsTr("%1Qt Quick Effect Maker ").arg(mainWindow.projectTitle)
    color: mainView.backgroundColor1
    menuBar: MenuBar {
        Menu {
            id: fileMenu
            title: qsTr("&File")
            Action {
                text: qsTr("&New Project")
                onTriggered: maybeNewProjectAction();
                enabled: !qdsMode
                shortcut: StandardKey.New
            }
            Action {
                text: qsTr("&Open Project")
                onTriggered: maybeOpenProjectAction();
                enabled: !qdsMode
                shortcut: StandardKey.Open
            }
            Menu {
                id: recentProjectsMenu
                title: "Recent Projects"
                enabled: !qdsMode && recentProjectsRepeater.count > 0
                Repeater {
                    id: recentProjectsRepeater
                    model: effectManager.settings.recentProjectsModel
                    MenuItem {
                        text: model.name
                        onTriggered: {
                            fileMenu.close();
                            maybeOpenProjectAction(model.file);
                        }
                    }
                }
            }
            Action {
                text: qsTr("Close Project")
                onTriggered: maybeCloseProjectAction();
                enabled: !qdsMode && effectManager.hasProjectFilename
                shortcut: StandardKey.Close
            }
            MenuSeparator { }
            Action {
                text: qsTr("&Save")
                onTriggered: saveProjectAction();
                shortcut: StandardKey.Save
            }
            Action {
                text: qsTr("Save &As...")
                onTriggered: saveProjectAsAction();
                enabled: !qdsMode && effectManager.hasProjectFilename
                shortcut: StandardKey.SaveAs
            }
            MenuSeparator { }
            Action {
                text: qsTr("&Export")
                onTriggered: exportAction();
                enabled: !qdsMode && effectManager.hasProjectFilename && effectManager.effectError.message === ""
                shortcut: "Ctrl+E"
            }
            Action {
                text: qsTr("Export As")
                onTriggered: exportAsAction();
                enabled: !qdsMode && effectManager.hasProjectFilename && effectManager.effectError.message === ""
            }
            MenuSeparator { }
            Action {
                text: qsTr("&Quit");
                onTriggered: maybeQuitAction();
                shortcut: StandardKey.Quit
            }
        }
        Menu {
            title: qsTr("&Edit")
            Action {
                text: qsTr("Undo");
                enabled: canUndo
                onTriggered: undoCodeAction();
            }
            Action {
                text: qsTr("Redo");
                enabled: canRedo
                onTriggered: redoCodeAction();
            }
            MenuSeparator { }
            Action {
                text: qsTr("Cut");
                enabled: canCut
                onTriggered: cutCodeAction();
            }
            Action {
                text: qsTr("Copy");
                enabled: canCopy
                onTriggered: copyCodeAction();
            }
            Action {
                text: qsTr("Paste");
                enabled: canPaste
                onTriggered: pasteCodeAction();
            }
            Action {
                text: qsTr("Find");
                enabled: canFind
                shortcut: StandardKey.Find
                onTriggered: showFindBarAction();
            }
            MenuSeparator { }
            Action {
                text: qsTr("Project Settings");
                onTriggered: projectSettingsAction();
            }
            Action {
                text: qsTr("Preferences");
                onTriggered: applicationSettingsAction();
            }
        }
        Menu {
            title: qsTr("&Node View")
            Action {
                text: qsTr("Add Node");
                onTriggered: addNodeAction();
            }
            Menu {
                title: "Selected"
                enabled: mainView.nodeViewItem.effectNodeSelected
                Action {
                    text: "Enable / Disable"
                    onTriggered: disableNodeAction();
                }
                Action {
                    text: "Rename"
                    onTriggered: renameNodeAction();
                }
                Action {
                    text: "Export"
                    onTriggered: exportNodeAction();
                }
                Action {
                    text: "Delete"
                    enabled: mainView.designMode && mainView.nodeViewItem.effectNodeSelected
                    onTriggered: deleteNodeAction();
                    shortcut: StandardKey.Delete
                }
            }
            MenuSeparator { }
            Action {
                text: qsTr("Clear");
                onTriggered: clearNodeViewAction();
            }
        }
        Menu {
            title: qsTr("&Tools")
            Action {
                text: qsTr("QSB Inspector");
                onTriggered: qsbInspectorAction();
            }
            Action {
                text: qsTr("Render to Image");
                onTriggered: renderToImageAction();
            }
        }
        Menu {
            title: qsTr("&Help")
            Action {
                text: qsTr("Quick Help")
                onTriggered: helpAction();
            }
            Action {
                text: qsTr("&About")
                onTriggered: aboutAction();
            }
        }
    }

    Menu {
        id: nodeContextMenu
        enabled: mainView.nodeViewItem.effectNodeSelected
        Action {
            text: "Enable / Disable"
            onTriggered: disableNodeAction();
        }
        Action {
            text: "Rename"
            onTriggered: renameNodeAction();
        }
        Action {
            text: "Export"
            onTriggered: exportNodeAction();
        }
        Action {
            text: "Delete"
            onTriggered: deleteNodeAction();
        }
    }

    FontLoader {
        id: codeFont
        source: effectManager.settings.codeFontFile
    }

    FileDialog {
        id: openEffectProjectDialog
        title: "Open an Effect Project File"
        nameFilters: ["Quick Effect Project (*.qep)"]
        onAccepted: {
            if (currentFile) {
                openProjectAction(currentFile);
            }
        }
    }

    FileDialog {
        id: saveProjectAsDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["Quick Effect Project (*.qep)"]
        onAccepted: {
            if (currentFile)
                effectManager.saveProject(currentFile)
        }
    }

    FileDialog {
        id: saveNodeAsDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["Quick Effect Node (*.qen)"]
        onAccepted: {
            if (currentFile)
                effectManager.saveSelectedNode(currentFile)
        }
    }

    FileDialog {
        id: saveImageAsDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["Portable Network Graphics (*.png)"]
        onAccepted: {
            if (currentFile)
                mainView.outputView.renderToImage(currentFile);
        }
    }

    SaveProjectDialog {
        id: saveProjectDialog
        parent: Overlay.overlay
        anchors.centerIn: parent

        function goToAction() {
            if (action === 0) {
                newProjectAction(true);
            } else if (action === 1) {
                openProjectAction(openFileName);
            } else if (action === 2) {
                closeProjectAction();
            } else if (action === 3) {
                quitAction();
            }
        }

        onAccepted: {
            // Save and then proceed to action
            effectManager.saveProject();
            goToAction();
        }
        onDiscarded: {
            // Discard saving, proceed to action directly
            goToAction();
            saveProjectDialog.close();
        }
        onRejected: {
            // Cancel the action
        }
    }

    AboutDialog {
        id: aboutDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }

    NewProjectDialog {
        id: newProjectDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }

    ExportEffectDialog {
        id: exportEffectDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }

    DeleteNodeDialog {
        id: deleteNodeDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }
    ClearNodeViewDialog {
        id: clearNodeViewDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }
    ProjectSettingsDialog {
        id: projectSettingsDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }
    ApplicationSettingsDialog {
        id: applicationSettingsDialog
    }
    QsbInspectorDialog {
        id: qsbInspectorDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }

    function saveProjectAction() {
        if (qdsMode) {
            saveAndExportProjectAction();
        } else {
            if (effectManager.hasProjectFilename)
                effectManager.saveProject();
            else
                newProjectAction(false);
        }
    }

    function saveAndExportProjectAction() {
        if (effectManager.hasProjectFilename) {
            effectManager.saveProject();
            mainWindow.exportAction();
        } else {
            newProjectDialog.exportNext = true;
            mainWindow.newProjectAction(false);
        }
    }

    function maybeOpenProjectAction(openFileName) {
        if (effectManager.hasProjectFilename && effectManager.unsavedChanges) {
            saveProjectDialog.action = 1;
            if (openFileName !== undefined)
                saveProjectDialog.openFileName = openFileName;
            else
                saveProjectDialog.openFileName = "";
            saveProjectDialog.open();
        } else {
            openProjectAction(openFileName);
        }
    }

    // When filename is given, opens that.
    // Otherwise opens file dialog to browse the file.
    function openProjectAction(openFileName) {
        if (openFileName !== undefined && openFileName !== "") {
            mainView.mainToolbar.setDesignModeInstantly(true);
            effectManager.loadProject(openFileName);
            return;
        }
        if (effectManager.projectDirectory) {
            openEffectProjectDialog.currentFolder = effectManager.projectDirectory;
        }
        openEffectProjectDialog.open();
    }

    function maybeCloseProjectAction() {
        if (effectManager.hasProjectFilename && effectManager.unsavedChanges) {
            saveProjectDialog.action = 2;
            saveProjectDialog.open();
        } else {
            closeProjectAction();
        }
    }

    function closeProjectAction() {
        effectManager.closeProject();
    }

    function maybeNewProjectAction() {
        if (effectManager.hasProjectFilename && effectManager.unsavedChanges) {
            saveProjectDialog.action = 0;
            saveProjectDialog.open();
        } else {
            newProjectAction(true);
        }
    }

    function newProjectAction(clearNodeView) {
        newProjectDialog.clearNodeView = clearNodeView;
        newProjectDialog.open();
    }

    function saveProjectAsAction() {
        if (effectManager.projectFilename) {
            saveProjectAsDialog.currentFolder = effectManager.addFileToURL(effectManager.projectDirectory);
            saveProjectAsDialog.currentFile = effectManager.projectFilename;
        }
        saveProjectAsDialog.open();
    }

    function maybeQuitAction() {
        if (effectManager.hasProjectFilename && effectManager.unsavedChanges) {
            saveProjectDialog.action = 3;
            saveProjectDialog.open();
        } else {
            quitAction();
        }
    }

    function quitAction() {
        mainWindow.quitAccepted = true;
        Qt.quit();
    }

    function addNodeAction() {
        mainView.nodeViewItem.showAddNodeDialog(-1, -1);
    }

    function renameNodeAction() {
        mainView.nodeViewItem.showRenameNodeDialog();
    }

    function disableNodeAction() {
        mainView.nodeViewItem.toggleNodeDisabled();
        effectManager.updateQmlComponent();
        effectManager.bakeShaders(true);
    }

    function exportNodeAction() {
        saveNodeAsDialog.currentFile = effectManager.nodeView.selectedNodeName + ".qen";
        saveNodeAsDialog.open();
    }
    function deleteNodeAction() {
        deleteNodeDialog.open();
    }

    function clearNodeViewAction() {
        clearNodeViewDialog.open();
    }

    function aboutAction() {
        aboutDialog.open();
    }

    function helpAction() {
        mainView.outputView.showHelp();
    }

    function exportAction() {
        if (effectManager.exportFilename !== "") {
            // Export with the default/latest settings
            // without showing the dialog.
            exportEffectDialog.initializeDialog();
            exportEffectDialog.exportEffect();
        } else {
            exportAsAction();
        }
    }

    function exportAsAction() {
        exportEffectDialog.open();
    }

    function addPropertyAction() {
        propertyEditDialog.propertyIndex = -1;
        propertyEditDialog.initialType = 2;
        propertyEditDialog.initialName = "";
        propertyEditDialog.initialDescription = "";
        // Default to false or 0.0
        propertyEditDialog.initialDefaultValue = false;
        propertyEditDialog.initialMinValue = "0.0";
        propertyEditDialog.initialMaxValue = "1.0";
        propertyEditDialog.initialUseCustomValue = false;
        propertyEditDialog.initialCustomValue = "";
        propertyEditDialog.initialEnableMipmap = false;
        propertyEditDialog.initialExportImage = true;
        propertyEditDialog.open();
    }

    function undoCodeAction() {
        currentEditorComponent.undo();
    }

    function redoCodeAction() {
        currentEditorComponent.redo();
    }

    function cutCodeAction() {
        currentEditorComponent.cut();
    }

    function copyCodeAction() {
        currentEditorComponent.copy();
    }

    function pasteCodeAction() {
        currentEditorComponent.paste();
    }

    function projectSettingsAction() {
        projectSettingsDialog.itemPadding = effectManager.effectPadding;
        projectSettingsDialog.newItemPadding = effectManager.effectPadding;
        projectSettingsDialog.open();
    }

    function applicationSettingsAction() {
        applicationSettingsDialog.show();
    }

    function qsbInspectorAction(qsbFile) {
        if (qsbFile !== undefined)
            qsbInspectorDialog.setQsbFile(qsbFile);
        qsbInspectorDialog.open();
    }

    function showFindBarAction() {
        mainView.showFindBar = true;
        mainView.currentEditorComponent.findAction();
    }

    function renderToImageAction() {
        if (saveImageAsDialog.currentFile == "") {
            saveImageAsDialog.currentFolder = effectManager.addFileToURL(effectManager.projectDirectory);
            saveImageAsDialog.currentFile = "effect_render.png";
        }
        saveImageAsDialog.open();
    }

    MainView {
        id: mainView
    }
}

