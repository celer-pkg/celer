// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtCore

Window {
    id: rootItem

    readonly property real baseMargin: 20

    title: qsTr("Preferences")
    width: 660
    height: 620
    minimumWidth: 400
    minimumHeight: 400
    color: mainView.backgroundColor1
    modality: Qt.ApplicationModal
    flags: Qt.Dialog

    function addPreviewSourceImage() {
        sourceImagesFileDialog.open();
    }

    FileDialog {
        id: sourceImagesFileDialog
        currentFolder: effectManager.getDefaultImagesDirectory()
        onAccepted: {
            if (selectedFile) {
                effectManager.settings.addSourceImage(selectedFile);
                mainView.outputView.effectPreview.selectLastPreviewSource();
            }
        }
    }

    FileDialog {
        id: codeFontFileDialog
        nameFilters: ["TrueType font (*.ttf)"]
        currentFolder: StandardPaths.writableLocation(StandardPaths.FontsLocation)
        onAccepted: {
            if (selectedFile) {
                effectManager.settings.setCodeFontFile(selectedFile);
            }
        }
    }

    FolderDialog {
        id: customNodePathDialog
        onAccepted: {
            if (currentFolder) {
                let added = effectManager.settings.addCustomNodesPath(currentFolder.toString());
                if (added)
                    effectManager.refreshAddNodesList();
            }
        }
    }

    ScrollView {
        id: scrollView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: footer.top
        contentWidth: mainContent.implicitWidth
        contentHeight: mainContent.implicitHeight + 2 * baseMargin

        Column {
            id: mainContent
            width: rootItem.width - 2 * baseMargin
            x: baseMargin
            y: baseMargin

            Item {
                id: sourceImagesToolbar
                width: parent.width
                height: 40
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: mainView.foregroundColor2
                    font.pixelSize: 14
                    font.bold: true
                    text: "Source Images"
                }
                Button {
                    height: parent.height - 4
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: addSourceImageButton.left
                    anchors.rightMargin: 5
                    text: qsTr("Refresh");
                    onClicked: {
                        effectManager.settings.refreshSourceImagesModel();
                    }
                }
                Button {
                    id: addSourceImageButton
                    height: parent.height - 4
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    text: qsTr("Add");
                    onClicked: {
                        sourceImagesFileDialog.open();
                    }
                }
            }

            Rectangle {
                id: sourceImagesTable
                width: parent.width
                height: 120
                color: mainView.backgroundColor2
                border.color: mainView.foregroundColor1
                border.width: 1
                ListView {
                    id: sourceImagesList
                    anchors.fill: parent
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        id: scrollBar
                        policy: ScrollBar.AlwaysOn
                    }
                    model: effectManager.settings.sourceImagesModel
                    delegate: Item {
                        width: sourceImagesList.width
                        height: 30
                        Row {
                            id: delegateRow
                            x: 10
                            width: parent.width - x * 2
                            height: parent.height
                            spacing: 10
                            Rectangle {
                                id: sourceImageItem
                                y: 4
                                width: 100
                                height: parent.height - 8
                                border.width: 1
                                border.color: mainView.foregroundColor1
                                color: mainView.backgroundColor1
                                Image {
                                    anchors.fill: parent
                                    anchors.margins: 1
                                    fillMode: Image.PreserveAspectCrop
                                    source: model.file
                                    mipmap: true
                                }
                            }
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - scrollBar.width - sourceImageItem.width - sizeItem.width - removeButton.width - delegateRow.spacing * 3
                                text: model.file
                                color: mainView.foregroundColor2
                                elide: Text.ElideLeft
                            }
                            Text {
                                id: sizeItem
                                anchors.verticalCenter: parent.verticalCenter
                                text: "(" + model.width + " x " + model.height + ")"
                                color: mainView.foregroundColor2
                                elide: Text.ElideLeft
                            }
                            CustomIconButton {
                                id: removeButton
                                anchors.verticalCenter: parent.verticalCenter
                                height: parent.height * 0.6
                                width: height
                                icon: "images/icon_remove_shadow.png"
                                description: "Remove"
                                enabled: model.canRemove
                                onClicked: {
                                    effectManager.settings.removeSourceImage(model.index);
                                }
                            }
                        }
                    }
                }
            }
            Item {
                width: 1
                height: 20
            }
            Item {
                id: customNodesToolbar
                width: parent.width
                height: 40
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: mainView.foregroundColor2
                    font.pixelSize: 14
                    font.bold: true
                    text: "Custom Nodes"
                }
                Button {
                    id: addCustomNodesPathButton
                    height: parent.height - 4
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    text: qsTr("Add");
                    onClicked: {
                        customNodePathDialog.open();
                    }
                }
            }
            Rectangle {
                id: customNodesTable
                width: parent.width
                height: 60
                color: mainView.backgroundColor2
                border.color: mainView.foregroundColor1
                border.width: 1
                ListView {
                    id: customNodesList
                    anchors.fill: parent
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        id: nodesListScrollBar
                        policy: ScrollBar.AlwaysOn
                    }
                    model: effectManager.settings.customNodesModel
                    delegate: Item {
                        width: customNodesList.width
                        height: 30
                        Row {
                            id: customNodesDelegateRow
                            x: 10
                            width: parent.width - x * 2
                            height: parent.height
                            spacing: 10
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - nodesListScrollBar.width - removeNodePathButton.width - customNodesDelegateRow.spacing
                                text: model.path
                                color: mainView.foregroundColor2
                                elide: Text.ElideLeft
                            }
                            CustomIconButton {
                                id: removeNodePathButton
                                anchors.verticalCenter: parent.verticalCenter
                                height: parent.height * 0.6
                                width: height
                                icon: "images/icon_remove_shadow.png"
                                description: "Remove"
                                onClicked: {
                                    effectManager.settings.removeCustomNodesPath(model.index);
                                }
                            }
                        }
                    }
                }
            }
            Item {
                width: 1
                height: 20
            }
            Row {
                height: 40
                spacing: 10
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: mainView.foregroundColor2
                    font.pixelSize: 14
                    font.bold: true
                    text: "Recent Projects menu"
                }
                Button {
                    height: parent.height - 4
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Clear");
                    onClicked: {
                        effectManager.settings.clearRecentProjectsModel();
                    }
                }
            }
            Item {
                width: 1
                height: 20
            }
            Column {
                width: parent.width
                CheckBox {
                    text: "Enable legacy GLSL versions"
                    checked: effectManager.settings.useLegacyShaders
                    onToggled: {
                        effectManager.settings.useLegacyShaders = checked;
                    }
                }
                Text {
                    width: parent.width
                    color: mainView.foregroundColor2
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    text: "This will include also OpenGL 2.1 and OpenGL ES 2.0 versions into baked shaders. Note: Some effect nodes do not work when the legacy mode is enabled."
                }
            }
            Item {
                width: 1
                height: 20
            }
            RowLayout {
                id: fontSelection
                width: parent.width
                height: 40
                Text {
                    color: mainView.foregroundColor2
                    font.pixelSize: 14
                    font.bold: true
                    text: "Source Font:"
                }
                Text {
                    Layout.fillWidth: true
                    elide: Text.ElideMiddle
                    text: effectManager.settings.codeFontFile
                    font.pixelSize: 14
                    font.italic: true
                    color: mainView.foregroundColor2
                }
                Button {
                    Layout.preferredHeight: 40
                    text: qsTr("Select");
                    onClicked: {
                        codeFontFileDialog.open();
                    }
                }
                ComboBox {
                    id: fontSizeSelector
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 40
                    model: [6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72]
                    onActivated: {
                        effectManager.settings.setCodeFontSize(currentValue);
                    }
                    Component.onCompleted: {
                        currentIndex = indexOfValue(effectManager.settings.codeFontSize);
                    }
                    Connections {
                        target: effectManager.settings
                        function onCodeFontSizeChanged() {
                            fontSizeSelector.currentIndex = fontSizeSelector.indexOfValue(effectManager.settings.codeFontSize);
                        }
                    }
                }
                CustomIconButton {
                    height: 20
                    width: height
                    icon: "images/icon_reset.png"
                    onClicked: {
                        effectManager.settings.resetCodeFont();
                    }
                }
            }
        }
    }

    DialogButtonBox {
        id: footer
        anchors.bottom: parent.bottom
        width: parent.width
        standardButtons: DialogButtonBox.Close
        onRejected: {
            rootItem.close();
        }
    }
}
