// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Pane {
    id: panel
    required property TorusGeometry target

    ColumnLayout {
        width: panel.width
        Label {
            text: "Torus Settings"
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Rings"
                Layout.fillWidth: true
            }
            SpinBox {
                from: 3
                to: 100
                value: panel.target.rings
                onValueChanged: panel.target.rings = value
                editable: true
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Segments"
                Layout.fillWidth: true
            }
            SpinBox {
                from: 0
                to: 100
                value: panel.target.segments
                onValueChanged: panel.target.segments = value
                editable: true
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Radius"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.radius
                onValueChanged: panel.target.radius = value
            }
            Label {
                text: panel.target.radius.toFixed(2)
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "tubeRadius"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.tubeRadius
                onValueChanged: panel.target.tubeRadius = value
            }
            Label {
                text: panel.target.tubeRadius.toFixed(2)
                Layout.minimumWidth: 50
            }
        }
    }
}
