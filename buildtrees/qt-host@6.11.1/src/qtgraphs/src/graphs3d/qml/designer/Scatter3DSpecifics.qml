// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import HelperWidgets
import QtQuick.Layouts

Column {
    anchors.left: parent.left
    anchors.right: parent.right

    Section {
        anchors.left: parent.left
        anchors.right: parent.right
        caption: qsTr("Scatter")

        SectionLayout {
            PropertyLabel {
                text: qsTr("Polar Coordinates")
                tooltip: qsTr("Use polar coordinates")
                Layout.fillWidth: true
            }
            SecondColumnLayout {
                CheckBox {
                    id: polarCheckbox
                    backendValue: backendValues.polar
                    Layout.fillWidth: true
                }
            }
            PropertyLabel {
                text: qsTr("Label Offset")
                tooltip: qsTr("Normalized horizontal radial label offset")
                Layout.fillWidth: true
                visible: polarCheckbox.checked
            }
            SecondColumnLayout {
                visible: polarCheckbox.checked
                SpinBox {
                    backendValue: backendValues.radialLabelOffset
                    minimumValue: 0.0
                    maximumValue: 1.0
                    stepSize: 0.01
                    decimals: 2
                    Layout.fillWidth: true
                }
            }
            PropertyLabel {
                text: qsTr("Selection Mode")
                tooltip: qsTr("Scatter item selection mode")
                Layout.fillWidth: true
            }
            SecondColumnLayout {
                ComboBox {
                    backendValue: backendValues.selectionMode
                    model: ["None", "Item"]
                    Layout.fillWidth: true
                    scope: "Graphs3D"
                }
            }
        }
    }

    GraphsSection {}

    GraphsCameraSection {}
}
