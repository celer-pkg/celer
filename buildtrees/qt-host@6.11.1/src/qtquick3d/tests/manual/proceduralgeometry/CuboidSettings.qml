// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Pane {
    id: panel
    required property CuboidGeometry target

    ColumnLayout {
        width: panel.width
        Label {
            text: "Cuboid Settings"
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "xExtent"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                // stepSize: 1
                value: panel.target.xExtent
                onValueChanged: panel.target.xExtent = value
            }
            Label {
                text: panel.target.xExtent.toFixed(1)
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "yExtent"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                // stepSize: 1
                value: panel.target.yExtent
                onValueChanged: panel.target.yExtent = value
            }
            Label {
                text: panel.target.yExtent.toFixed(1)
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "zExtent"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                // stepSize: 1
                value: panel.target.zExtent
                onValueChanged: panel.target.zExtent = value
            }
            Label {
                text: panel.target.zExtent.toFixed(1)
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "YZ Mesh Resolution: X"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.yzMeshResolution.width
                onValueChanged: panel.target.yzMeshResolution.width = value
            }
            Label {
                text: panel.target.yzMeshResolution.width
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "YZ Mesh Resolution: Y"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.yzMeshResolution.height
                onValueChanged: panel.target.yzMeshResolution.height = value
            }
            Label {
                text: panel.target.yzMeshResolution.height
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "XZ Mesh Resolution: X"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.xzMeshResolution.width
                onValueChanged: panel.target.xzMeshResolution.width = value
            }
            Label {
                text: panel.target.xzMeshResolution.width
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "XZ Mesh Resolution: Y"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.xzMeshResolution.height
                onValueChanged: panel.target.xzMeshResolution.height = value
            }
            Label {
                text: panel.target.xzMeshResolution.height
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "XY Mesh Resolution: X"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.xyMeshResolution.width
                onValueChanged: panel.target.xyMeshResolution.width = value
            }
            Label {
                text: panel.target.xyMeshResolution.width
                Layout.minimumWidth: 50
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "XY Mesh Resolution: Y"
            }
            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: panel.target.xyMeshResolution.height
                onValueChanged: panel.target.xyMeshResolution.height = value
            }
            Label {
                text: panel.target.xyMeshResolution.height
                Layout.minimumWidth: 50
            }
        }

    }
}
