// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QQEMLib 1.0

Item {
    id: mainView

    property alias effectManager: effectManager
    property alias outputView: outputView
    property alias outputEditorView: outputView.outputEditorView
    property alias nodeViewItem: nodeViewItem
    property alias propertyEditDialog: propertyEditDialog
    property alias currentEditorComponent: codeEditorView.currentEditorComponent
    property alias mainToolbar: mainToolbar
    property color highlightColor: "#2aafd3"
    property color backgroundColor1: "#1f1f1f"
    property color backgroundColor2: "#2e2f30"
    property color foregroundColor1: "#606060"
    property color foregroundColor2: "#dadada"
    property bool previewAnimationRunning: false
    // True = design mode, false = coding mode.
    property alias designMode: mainToolbar.designMode
    property alias designModeAnimated: mainToolbar.designModeAnimated
    property bool switchDesignModeInstantly: false
    // Initial sizes, can be adjusted by user
    property real designViewWidth: 0.3
    property real codeViewWidth: 0.6
    property real currentLeftViewWidth: designMode ? designViewWidth : codeViewWidth
    property bool showFindBar: false

    anchors.fill: parent

    function updateShaders(forceUpdate) {
        nodeViewItem.selectedNodeQmlCode = codeEditorView.qmlText;
        nodeViewItem.selectedNodeFragmentCode = codeEditorView.fragmentText;
        nodeViewItem.selectedNodeVertexCode = codeEditorView.vertexText;
        effectManager.updateQmlComponent();
        effectManager.bakeShaders(forceUpdate);
    }

    Behavior on currentLeftViewWidth {
        enabled: !mainSplitView.resizing && !switchDesignModeInstantly
        NumberAnimation {
            duration: 400
            easing.type: Easing.InOutQuad
        }
    }

    PropertyEditDialog {
        id: propertyEditDialog
    }

    EffectManager {
        id: effectManager
        nodeView: nodeViewItem
        vertexShader: outputEditorView.vertexEditor.text
        fragmentShader: outputEditorView.fragmentEditor.text
        onVertexShaderChanged: {
            outputEditorView.vertexEditor.text = effectManager.vertexShader
        }
        onFragmentShaderChanged: {
            outputEditorView.fragmentEditor.text = effectManager.fragmentShader
        }
        onQmlComponentStringChanged: {
            outputEditorView.qmlEditor.text = effectManager.qmlComponentString
        }

        Component.onCompleted: {
            // Initialize
            effectManager.bakeShaders(true);
        }
    }

    CustomSplitView {
        id: mainSplitView
        anchors.fill: parent
        orientation: Qt.Horizontal
        Item {
            SplitView.preferredWidth: mainView.width * currentLeftViewWidth
            SplitView.minimumWidth: mainView.width * 0.2
            SplitView.maximumWidth: mainView.width * 0.8
            clip: true
            onWidthChanged: {
                if (mainSplitView.resizing) {
                    // User is moving the SplitView separator, so store
                    // the updated width into current mode.
                    var newWidth = width / mainSplitView.width;
                    if (designMode)
                        designViewWidth = newWidth;
                    else
                        codeViewWidth = newWidth;
                }
            }

            MainToolbar {
                id: mainToolbar
                width: parent.width
                height: 50
                z: 2
            }

            NodeView {
                id: nodeViewItem
                anchors.top: mainToolbar.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                opacity: designModeAnimated
                visible: opacity && initialized
            }
            Item {
                anchors.top: mainToolbar.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                opacity: 1.0 - designModeAnimated
                visible: opacity
                EditorView {
                    id: codeEditorView
                    anchors.fill: parent
                    fragmentText: nodeViewItem.selectedNodeFragmentCode
                    onFragmentTextChanged: {
                        shaderUpdateTimer.restart();
                    }
                    vertexText: nodeViewItem.selectedNodeVertexCode
                    onVertexTextChanged: {
                        shaderUpdateTimer.restart();
                    }
                    qmlText: nodeViewItem.selectedNodeQmlCode
                    onQmlTextChanged: {
                        shaderUpdateTimer.restart();
                    }
                    visible: nodeViewItem.effectNodeSelected || nodeViewItem.mainNodeSelected
                    Timer {
                        id: shaderUpdateTimer
                        interval: 1
                        onTriggered: {
                            // Update with a small delay to synchronize vs & fs updates
                            updateShaders(false);
                        }
                    }
                }
                Text {
                    anchors.centerIn: parent
                    font.pixelSize: 14
                    color: mainView.foregroundColor1
                    text: "Select an effect node to see its code."
                    visible: !codeEditorView.visible
                }
            }
        }

        CustomSplitView {
            SplitView.fillWidth: true
            orientation: Qt.Vertical
            clip: true
            PropertiesView {
                id: propertiesView
                SplitView.preferredHeight: 200
            }
            OutputView {
                id: outputView
                SplitView.fillHeight: true
                SplitView.minimumHeight: 48
            }
        }
    }
}
