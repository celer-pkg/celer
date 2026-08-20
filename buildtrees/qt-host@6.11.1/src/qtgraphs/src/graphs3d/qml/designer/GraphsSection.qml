// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import HelperWidgets
import QtQuick.Layouts

Section {
    anchors.left: parent.left
    anchors.right: parent.right
    caption: qsTr("Graph")

    SectionLayout {

        PropertyLabel {
            text: qsTr("Render Mode")
            tooltip: qsTr("Rendering mode")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            ComboBox {
                backendValue: backendValues.renderingMode
                model: ["Indirect", "DirectToBackground"]
                Layout.fillWidth: true
                scope: "Graphs3D"
            }
        }
        PropertyLabel {
            text: qsTr("Shadow Quality")
            tooltip: qsTr("Quality and style of the shadows")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            ComboBox {
                backendValue: backendValues.shadowQuality
                model: ["None", "Low", "Medium",
                    "High", "SoftLow", "SoftMedium",
                    "SoftHigh"]
                Layout.fillWidth: true
                scope: "Graphs3D"
            }
        }
        PropertyLabel {
            text: qsTr("Optimization")
            tooltip: qsTr("Optimization hint")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            ComboBox {
                backendValue: backendValues.optimizationHint
                model: ["Default", "Legacy"]
                Layout.fillWidth: true
                scope: "Graphs3D"
            }
        }
        PropertyLabel {
            text: qsTr("MSAA")
            tooltip: qsTr("Multisample anti-aliasing sample count")
            Layout.fillWidth: true
        }
        SpinBox {
            backendValue: backendValues.msaaSamples
            minimumValue: 0
            maximumValue: 8
            Layout.fillWidth: true
        }
        PropertyLabel {
            text: qsTr("Aspect Ratio")
            tooltip: qsTr("Horizontal to vertical aspect ratio")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.aspectRatio
                minimumValue: 0.1
                maximumValue: 10.0
                stepSize: 0.1
                decimals: 1
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Horizontal AR")
            tooltip: qsTr("Horizontal aspect ratio")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.horizontalAspectRatio
                minimumValue: 0.1
                maximumValue: 10.0
                stepSize: 0.1
                decimals: 1
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Margin")
            tooltip: qsTr("Graph background margin")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.margin
                minimumValue: -1.0
                maximumValue: 100.0
                stepSize: 0.1
                decimals: 1
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Measure FPS")
            tooltip: qsTr("Measure rendering speed as Frames Per Second")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            CheckBox {
                backendValue: backendValues.measureFps
                Layout.fillWidth: true
            }
        }
    }
}

