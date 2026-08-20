// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Pane {
    id: panel
    required property PlaneGeometry target

    ColumnLayout {
        width: panel.width
        Label {
            text: "Plane Settings"
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "width"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                // stepSize: 1
                value: panel.target.width
                onValueChanged: panel.target.width = value
            }
            Label {
                text: panel.target.width.toFixed(1)
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "height"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                // stepSize: 1
                value: panel.target.height
                onValueChanged: panel.target.height = value
            }
            Label {
                text: panel.target.height.toFixed(1)
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Mesh Resolution: X"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.meshResolution.width
                onValueChanged: panel.target.meshResolution.width = value
            }
            Label {
                text: panel.target.meshResolution.width
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Mesh Resolution: Y"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.meshResolution.height
                onValueChanged: panel.target.meshResolution.height = value
            }
            Label {
                text: panel.target.meshResolution.height
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Plane"
            }
            ComboBox {
                id: planeComboBox
                textRole: "text"
                valueRole: "value"
                implicitContentWidthPolicy: ComboBox.WidestText
                onActivated: panel.target.plane = currentValue
                Component.onCompleted: currentIndex = indexOfValue(panel.target.plane)
                Layout.alignment: Qt.AlignRight
                model: [
                    { value: PlaneGeometry.XY, text: "XY" },
                    { value: PlaneGeometry.XZ, text: "XZ"},
                    { value: PlaneGeometry.ZY, text: "ZY"}
                ]

                Connections {
                    target: panel.target
                    function onPlaneChanged() {
                        planeComboBox.currentIndex = planeComboBox.indexOfValue(panel.target.plane)
                    }
                }
            }
        }
        CheckBox {
            id: reversedCheckbox
            text: "Reversed"
            checked: panel.target.reversed
            onCheckedChanged: panel.target.reversed = checked
        }
        CheckBox {
            id: mirroredCheckbox
            text: "Mirrored (V)"
            checked: panel.target.mirrored
            onCheckedChanged: panel.target.mirrored = checked
        }

    }
}
