// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Dialogs
import QtCore
import QQEMLib 1.0

CustomDialog {
    id: root

    function setQsbFile(qsbFile) {
        qsbInspectorHelper.loadQsb(qsbFile);
    }

    title: qsTr("QSB Inspector")
    width: 800
    height: 680
    modal: true
    focus: true
    standardButtons: Dialog.Close
    closePolicy: Popup.NoAutoClose

    QsbInspectorHelper {
        id: qsbInspectorHelper
    }

    FileDialog {
        id: qsbFileDialog
        title: "Open an QSB File"
        nameFilters: ["Qt Shader Baker File (*.qsb)"]
        onAccepted: {
            if (currentFile)
                qsbInspectorHelper.loadQsb(currentFile);
        }
    }

    GridLayout {
        id: detailsArea
        width: parent.width
        columns: 4
        columnSpacing: 10
        rowSpacing: 0
        Text {
            text: qsTr("FILE:")
            font.bold: true
            font.pixelSize: 14
            color: mainView.foregroundColor2
        }
        Text {
            id: fileTextEdit
            Layout.fillWidth: true
            text: qsbInspectorHelper.shaderData.currentFile
            font.pixelSize: 16
            color: mainView.foregroundColor1
            elide: Text.ElideLeft
            MouseArea {
                id: fileMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
            ToolTip {
                parent: fileTextEdit
                visible: fileMouseArea.containsMouse && text
                delay: 1000
                text: fileTextEdit.text
            }
        }
        Button {
            Layout.preferredHeight: 40
            text: qsTr("Browse");
            onClicked: {
                if (qsbInspectorHelper.shaderData.currentFile !== "") {
                    qsbFileDialog.currentFolder = effectManager.getDirectory(qsbInspectorHelper.shaderData.currentFile);
                    qsbFileDialog.currentFile = effectManager.addFileToURL(qsbInspectorHelper.shaderData.currentFile);
                }
                qsbFileDialog.open();
            }
        }
        ComboBox {
            Layout.preferredWidth: 160
            Layout.preferredHeight: 40
            textRole: "name"
            valueRole: "sourceIndex"
            enabled: count > 0
            model: qsbInspectorHelper.sourceSelectorModel
            onCurrentValueChanged: {
                qsbInspectorHelper.currentSourceIndex = currentValue;
            }
        }
    }
    Item {
        id: codeArea
        width: parent.width
        anchors.top: detailsArea.bottom
        anchors.bottom: parent.bottom
        CodeEditor {
            id: shaderSourceEdit
            anchors.fill: parent
            showLineNumbers: true
            editable: false
            contentType: 0
            text: qsbInspectorHelper.currentSourceCode
            visible: text != ""
        }
        Rectangle {
            anchors.fill: shaderSourceEdit
            color: "#40000000"
            border.width: 1
            border.color: mainView.foregroundColor1
            z: -1
            visible: shaderSourceEdit.visible
        }
        Column {
            id: infoTexts
            width: parent.width
            visible: !shaderSourceEdit.visible && qsbInspectorHelper.shaderData.currentFile !== ""

            // Returns more informative QSB version text
            function qsbToQtVersion(qsbVersion) {
                if (qsbVersion === 5) {
                    return qsbVersion + " (Qt 6.0 - 6.3)";
                } else if (qsbVersion === 6) {
                    return qsbVersion + " (Qt 6.4)";
                } else if (qsbVersion === 8) {
                    return qsbVersion + " (Qt 6.5 or newer)";
                }
                return qsbVersion;
            }

            Text {
                font.pixelSize: 14
                color: mainView.foregroundColor2
                text: "<b>QSB VERSION: </b>" + infoTexts.qsbToQtVersion(qsbInspectorHelper.shaderData.qsbVersion)
            }
            Text {
                font.pixelSize: 14
                color: mainView.foregroundColor2
                text: "<b>TYPE: </b>" + qsbInspectorHelper.shaderData.stage
            }
            Text {
                font.pixelSize: 14
                color: mainView.foregroundColor2
                text: "<b>SHADERS: </b>" + qsbInspectorHelper.shaderData.shaderCount
            }
            Text {
                font.pixelSize: 14
                color: mainView.foregroundColor2
                text: "<b>SIZE: </b>" + (qsbInspectorHelper.shaderData.size / 1000).toFixed(1) + " kb"
            }
        }
        Item {
            width: parent.width
            anchors.top: infoTexts.bottom
            anchors.topMargin: 10
            anchors.bottom: parent.bottom
            visible: infoTexts.visible && qsbInspectorHelper.shaderData.reflectionInfo !== ""
            Text {
                id: reflectionInfoText
                font.pixelSize: 14
                color: mainView.foregroundColor2
                text: "<b>REFLECTION INFO:</b>"
            }
            CodeEditor {
                id: reflectionInfoTextEdit
                width: parent.width
                anchors.top: reflectionInfoText.bottom
                anchors.topMargin: 8
                anchors.bottom: parent.bottom
                showLineNumbers: false
                editable: false
                contentType: 0
                text: qsbInspectorHelper.shaderData.reflectionInfo
            }
            Rectangle {
                anchors.fill: reflectionInfoTextEdit
                color: "#40000000"
                border.width: 1
                border.color: mainView.foregroundColor1
                z: -1
            }
        }
    }
}
