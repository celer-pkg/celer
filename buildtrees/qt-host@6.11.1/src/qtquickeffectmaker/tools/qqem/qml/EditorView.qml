// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: editorView

    property alias qmlEditor: qmlEdit
    property alias vertexEditor: vertEdit
    property alias fragmentEditor: fragEdit
    property var currentEditorComponent: tabBarEditors.currentIndex == 0
                                         ? qmlEdit.textArea :
                                           tabBarEditors.currentIndex == 1
                                           ? vertEdit.textArea : fragEdit.textArea
    property alias qmlText: qmlEdit.text
    property alias vertexText: vertEdit.text
    property alias fragmentText: fragEdit.text
    property bool showLineNumbers: true
    property bool editable: true

    function showErrorCodeLine() {
        // UI combines QML error types
        let tabIndex = (effectManager.effectError.type === 0 || effectManager.effectError.type === 3) ? 0 : effectManager.effectError.type
        tabIndex = Math.max(0, Math.min(2, tabIndex));
        tabBarEditors.currentIndex = tabIndex;
        const codeLine = effectManager.effectError.line;
        if (tabIndex === 0)
            qmlEditor.scrollToLine(codeLine);
        else if (tabIndex === 1)
            vertexEditor.scrollToLine(codeLine);
        else
            fragmentEditor.scrollToLine(codeLine);
    }

    ColumnLayout {
        width: parent.width
        height: parent.height
        Item {
            id: tabBarItem
            Layout.fillWidth: true
            height: tabBarEditors.height
            Rectangle {
                anchors.fill: tabBarItem
                color: backgroundColor1
            }
            TabBar {
                id: tabBarEditors
                currentIndex: 2
                TabButton {
                    id: qmlTabText
                    text: "QML"
                    width: 80
                    opacity: qmlText.length === 0 ? 0.3 : 1.0
                }
                TabButton {
                    id: vertexTabText
                    text: "VERT"
                    width: 80
                    opacity: vertexText.length === 0 ? 0.3 : 1.0
                }
                TabButton {
                    id: fragTabtext
                    text: "FRAG"
                    width: 80
                    opacity: fragmentText.length === 0 ? 0.3 : 1.0
                }
            }
            Rectangle {
                anchors.fill: editToolbar
                color: mainView.backgroundColor1
                opacity: 0.8
            }
            Row {
                id: editToolbar
                visible: editorView.editable
                anchors.right: parent.right
                height: tabBarEditors.height
                CustomIconButton {
                    id: autoplayButton
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height * 0.8
                    width: height
                    icon: "images/icon_autoplay.png"
                    toggledIcon: "images/icon_autoplay_on.png"
                    description: "Autoplay when the shader changes"
                    toggleButton: true
                    toggled: effectManager.autoPlayEffect
                    onClicked: {
                        effectManager.autoPlayEffect = !effectManager.autoPlayEffect;
                        if (effectManager.autoPlayEffect)
                            updateShaders(true);
                    }
                }
                CustomIconButton {
                    id: playButton
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height * 0.8
                    width: height
                    icon: "images/icon_play.png"
                    description: "Play the shader"
                    onClicked: {
                        updateShaders(true);
                    }
                    Rectangle {
                        anchors.fill: playButton
                        anchors.margins: 2
                        z: -1
                        color: effectManager.shadersUpToDate ? "transparent" : "#912c2c"
                        radius: width / 2
                    }
                }
                CustomIconButton {
                    id: exportButton
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height * 0.6
                    width: height
                    icon: "images/icon_export.png"
                    description: "Export the effect"
                    enabled: (effectManager.hasProjectFilename && effectManager.effectError.message === "")
                    onClicked: {
                        mainWindow.exportAction();
                    }
                }
            }
            Row {
                id: previewToolbar
                visible: !editToolbar.visible
                anchors.right: parent.right
                height: tabBarEditors.height
                CustomIconButton {
                    id: showQsbButton
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height * 0.8
                    width: height * (288 / 128)
                    icon: "images/button_qsb.png"
                    description: "Show QSB Inspector"
                    enabled: (tabBarEditors.currentIndex === 1 || tabBarEditors.currentIndex === 2)
                    onClicked: {
                        if (tabBarEditors.currentIndex === 1)
                            qsbInspectorAction(effectManager.vertexShaderFilename);
                        else
                            qsbInspectorAction(effectManager.fragmentShaderFilename);
                    }
                }
            }
        }

        // Editors
        Item {
            id: editorStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            MouseArea {
                anchors.fill: parent
                onWheel: {}
            }
            CodeEditor {
                id: qmlEdit
                anchors.fill: parent
                showLineNumbers: editorView.showLineNumbers
                editable: editorView.editable
                contentType: 1
                editorIndex: 0
                visible: tabBarEditors.currentIndex === 0
            }
            CodeEditor {
                id: vertEdit
                anchors.fill: parent
                showLineNumbers: editorView.showLineNumbers
                editable: editorView.editable
                contentType: 0
                editorIndex: 1
                visible: tabBarEditors.currentIndex === 1
            }
            CodeEditor {
                id: fragEdit
                anchors.fill: parent
                showLineNumbers: editorView.showLineNumbers
                editable: editorView.editable
                contentType: 0
                editorIndex: 2
                visible: tabBarEditors.currentIndex === 2
            }
        }
        FindBar {
            id: findBar
            show: mainView.showFindBar && editorView.editable
        }
    }
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: editable ? 0.4 : 0.9
        z: -1
    }
}
