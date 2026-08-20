// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import HelperWidgets
import QtQuick.Layouts

Section {
    anchors.left: parent.left
    anchors.right: parent.right
    caption: qsTr("Camera")

    SectionLayout {
        PropertyLabel {
            text: qsTr("Preset")
            tooltip: qsTr("Camera preset")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            ComboBox {
                backendValue: backendValues.cameraPreset
                model: ["NoPreset", "FrontLow", "Front", "FrontHigh", "LeftLow",
                    "Left", "LeftHigh", "RightLow", "Right", "RightHigh", "BehindLow",
                    "Behind", "BehindHigh", "IsometricLeft", "IsometricLeftHigh",
                    "IsometricRight", "IsometricRightHigh", "DirectlyAbove",
                    "DirectlyAboveCW45", "DirectlyAboveCCW45", "FrontBelow",
                    "LeftBelow", "RightBelow", "BehindBelow", "DirectlyBelow"]
                Layout.fillWidth: true
                scope: "Graphs3D"
            }
        }
        PropertyLabel {
            text: qsTr("Target")
            tooltip: qsTr("Camera target position")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.cameraTargetPosition_x
                minimumValue: -1.0
                maximumValue: 1.0
                stepSize: 0.01
                decimals: 2
                Layout.fillWidth: true
            }
            ControlLabel {
                text: "X"
            }

            SpinBox {
                backendValue: backendValues.cameraTargetPosition_y
                minimumValue: -1.0
                maximumValue: 1.0
                stepSize: 0.01
                decimals: 2
                Layout.fillWidth: true
            }
            ControlLabel {
                text:"Y"
            }

            SpinBox {
                backendValue: backendValues.cameraTargetPosition_z
                minimumValue: -1.0
                maximumValue: 1.0
                stepSize: 0.01
                decimals: 2
                Layout.fillWidth: true
            }
            ControlLabel {
                text: "Z"
            }
        }
        PropertyLabel {
            text: qsTr("Zoom")
            tooltip: qsTr("Camera zoom level")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.cameraZoomLevel
                minimumValue: backendValues.minCameraZoomLevel
                maximumValue: backendValues.maxCameraZoomLevel
                stepSize: 1
                decimals: 0
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Min Zoom")
            tooltip: qsTr("Camera minimum zoom")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.minCameraZoomLevel
                minimumValue: 0
                maximumValue: backendValues.maxCameraZoomLevel
                stepSize: 1
                decimals: 0
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Max Zoom")
            tooltip: qsTr("Camera maximum zoom")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.maxCameraZoomLevel
                minimumValue: backendValues.minCameraZoomLevel
                maximumValue: 500
                stepSize: 1
                decimals: 0
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("X Rotation")
            tooltip: qsTr("Camera X rotation")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.cameraXRotation
                minimumValue: -180
                maximumValue: 180
                stepSize: 1
                decimals: 0
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Wrap X")
            tooltip: qsTr("Wrap camera X rotation")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            CheckBox {
                backendValue: backendValues.wrapCameraXRotation
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Y Rotation")
            tooltip: qsTr("Camera Y rotation")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            SpinBox {
                backendValue: backendValues.cameraYRotation
                minimumValue: 0
                maximumValue: 90
                stepSize: 1
                decimals: 0
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Wrap Y")
            tooltip: qsTr("Wrap camera Y rotation")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            CheckBox {
                backendValue: backendValues.wrapCameraYRotation
                Layout.fillWidth: true
            }
        }
        PropertyLabel {
            text: qsTr("Orthographic")
            tooltip: qsTr("Use orthographic camera")
            Layout.fillWidth: true
        }
        SecondColumnLayout {
            CheckBox {
                backendValue: backendValues.orthoProjection
                Layout.fillWidth: true
            }
        }
    }
}
