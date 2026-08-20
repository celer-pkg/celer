// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Dialogs

Pane {
    id: panel
    required property ExtrudedTextGeometry target

    ColumnLayout {
        width: panel.width
        Label {
            text: "ExtrudedText Settings"
        }

        RowLayout {
            spacing: 10
            Label {
                text: "Text"
            }
            TextField {
                id: textInput
                text: panel.target.text
                onTextChanged: panel.target.text = textInput.text
            }
        }

        FontDialog {
            id: fontDialog
            currentFont: panel.target.font
            onSelectedFontChanged: {
                panel.target.font = fontDialog.selectedFont
            }
        }

        Label {
            text: `Family: ${panel.target.font.family}`
        }

        Button {
            id: fontDialogButton
            text: "Choose Font"
            onClicked: {
                fontDialog.open()
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Point Size"
            }
            Slider {
                Layout.fillWidth: true
                from: 1
                to: 100
                stepSize: 1
                value: panel.target.font.pointSize
                onValueChanged: panel.target.font.pointSize = value
            }
            Label {
                text: panel.target.font.pointSize
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Depth"
            }
            Slider {
                Layout.fillWidth: true
                from: 0.01
                to: 10
                value: panel.target.depth
                onValueChanged: panel.target.depth = value
            }
            Label {
                text: panel.target.depth.toFixed(2)
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Scale"
            }
            Slider {
                Layout.fillWidth: true
                from: 1
                to: 100
                value: panel.target.scale
                onValueChanged: panel.target.scale = value
            }
            Label {
                text: panel.target.scale.toFixed(2)
                Layout.minimumWidth: 50
            }
        }
    }
}
