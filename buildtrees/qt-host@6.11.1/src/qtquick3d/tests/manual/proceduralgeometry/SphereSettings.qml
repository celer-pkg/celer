// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Pane {
    id: panel
    required property SphereGeometry target

    ColumnLayout {
        width: panel.width
        Label {
            text: "Sphere Settings"
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "rings"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
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
                text: "segments"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.segments
                onValueChanged: panel.target.segments = value
            }
            Label {
                text: panel.target.segments
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "radius"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                value: panel.target.radius
                onValueChanged: panel.target.radius = value
            }
            Label {
                text: panel.target.radius.toFixed(2)
                Layout.minimumWidth: 50
            }
        }
    }
}
