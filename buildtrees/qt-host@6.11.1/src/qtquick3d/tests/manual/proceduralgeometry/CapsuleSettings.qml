
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Pane {
    id: panel
    required property CapsuleGeometry target

    ColumnLayout {
        width: panel.width
        Label {
            text: "Capsule Settings"
        }

        RowLayout {
            Layout.fillWidth: true
            CheckBox {
                Layout.fillWidth: true
                checked: panel.target.enableNormals
                text: qsTr("enableNormals")
                onCheckedChanged: panel.target.enableNormals = checked
            }
        }

        RowLayout {
            Layout.fillWidth: true
            CheckBox {
                Layout.fillWidth: true
                checked: panel.target.enableUV
                text: qsTr("enableUV")
                onCheckedChanged: panel.target.enableUV = checked
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "longitudes"
            }
            Slider {
                Layout.fillWidth: true
                from: 1
                to: 100
                stepSize: 1
                value: panel.target.longitudes
                onValueChanged: panel.target.longitudes = value
            }
            Label {
                text: panel.target.longitudes
                Layout.minimumWidth: 50
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "latitudes"
            }
            Slider {
                Layout.fillWidth: true
                from: 1
                to: 100
                stepSize: 1
                value: panel.target.latitudes
                onValueChanged: panel.target.latitudes = value
            }
            Label {
                text: panel.target.latitudes
                Layout.minimumWidth: 50
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "rings"
            }
            Slider {
                Layout.fillWidth: true
                from: 1
                to: 100
                stepSize: 1
                value: panel.target.rings
                onValueChanged: panel.target.rings = value
            }
            Label {
                text: panel.target.rings
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
                value: panel.target.height
                onValueChanged: panel.target.height = value
            }
            Label {
                text: panel.target.height.toFixed(2)
                Layout.minimumWidth: 50
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "diameter"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                value: panel.target.diameter
                onValueChanged: panel.target.diameter = value
            }
            Label {
                text: panel.target.diameter.toFixed(2)
                Layout.minimumWidth: 50
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "uvProfile"
            }
            ComboBox {
                textRole: "text"
                valueRole: "value"
                // When an item is selected, update the backend.
                onActivated: panel.target.uvProfile = currentValue
                // Set the initial currentIndex to the value stored in the backend.
                Component.onCompleted: currentIndex = indexOfValue(panel.target.uvProfile)
                model: [{
                        "value": CapsuleGeometry.Fixed,
                        "text": qsTr("Fixed")
                    }, {
                        "value": CapsuleGeometry.Aspect,
                        "text": qsTr("Aspect")
                    }, {
                        "value": CapsuleGeometry.Uniform,
                        "text": qsTr("Uniform")
                    }]
            }
        }
    }
}
