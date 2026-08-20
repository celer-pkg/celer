// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtGraphs
import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import AerospaceHubExample

Rectangle {
    id: smallgraph

    property PieSimulator piesimulator
    property BarSimulator barsimulator
    property LineSimulator linesimulator

    color: "#80000000"
    border.color: "#EFEFEF"
    border.width: 1
    radius: GlobalSettings.defaultradius

    GraphHeaderItem {
        id: header
        spacing: GlobalSettings.defaultspacing
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: spacing
        anchors.leftMargin: spacing * 1.5
        Layout.preferredHeight: GlobalSettings.dashboardgraphheaderheight
        titlesize: GlobalSettings.fontsize18px
        descsize: GlobalSettings.fontsize14px
        small: true
    }

    Bar2DGraph {
        id: bar2d
        anchors.topMargin: header.height + GlobalSettings.defaultspacing
        anchors.rightMargin: GlobalSettings.defaultspacing
        anchors.fill: parent
        opacity: 0
        barsimulator: smallgraph.barsimulator
    }

    Area2DGraph {
        id: area2d
        anchors.topMargin: header.height + GlobalSettings.defaultspacing
        anchors.rightMargin: GlobalSettings.defaultspacing
        anchors.fill: parent
        opacity: 0
        linesimulator: smallgraph.linesimulator
    }

    //! [previewgraphs]
    Pie2DGraph {
        id: pie2d
        anchors.topMargin: header.height + GlobalSettings.defaultspacing
        anchors.rightMargin: GlobalSettings.defaultspacing
        anchors.fill: parent
        opacity: 0
        piesimulator: smallgraph.piesimulator
    }

    Bar3DGraph {
        id: bar3d
        anchors.topMargin: header.height + GlobalSettings.defaultspacing
        anchors.bottomMargin: GlobalSettings.defaultspacing
        anchors.fill: parent
        opacity: 0
        cameraZoomLevel: GlobalSettings.defaultzoom
        cameraPreset: GlobalSettings.defaultpreset
    }
    //! [previewgraphs]

    Scatter3DGraph {
        id: scatter3d
        anchors.topMargin: header.height + GlobalSettings.defaultspacing
        anchors.bottomMargin: GlobalSettings.defaultspacing
        anchors.fill: parent
        opacity: 0
        cameraZoomLevel: GlobalSettings.defaultzoom
        cameraPreset: GlobalSettings.defaultpreset
    }

    Surface3DGraph {
        id: surface3d
        anchors.topMargin: header.height + GlobalSettings.defaultspacing
        anchors.bottomMargin: GlobalSettings.defaultspacing
        anchors.fill: parent
        opacity: 0
        cameraZoomLevel: GlobalSettings.defaultzoom
        cameraPreset: GlobalSettings.defaultpreset
    }

    // Darken the preview graph with an overlaid semitransparent black rectangle
    Rectangle {
        id: control
        anchors.fill: parent
        radius: GlobalSettings.defaultradius
        color: "#55000000"
        opacity: 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                easing.type: Easing.InOutCubic
                duration: 500
            }
        }
    }

    // Put a mouse area on a preview graph for handling hover to show the details and
    // settings buttons and handle mouse clicks on any of them
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            if (root.state === "ShowSettings")
                return
            root.settingsstate = "Show" + smallgraph.state
            root.state = "Show" + smallgraph.state
        }
        onContainsMouseChanged: {
            control.opacity = containsMouse ? 1 : 0
        }

        Button {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: GlobalSettings.defaultspacing * 1.5
            height: width
            icon.source: "../images/settings-icon.png"
            icon.color: "#EFEFEF"
            background: Rectangle {
                color: "#55000000"
                border.color: "#EFEFEF"
                border.width: parent.hovered ? 1 : 0
                radius: width / 5
            }
            onClicked: {
                if (root.state === "ShowSettings")
                    return
                root.settingsstate = "Show" + smallgraph.state
                root.state = "ShowSettings"
            }
            opacity: control.opacity
        }

        Button {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: parent.height / 2 + GlobalSettings.defaultspacing
            text: "Click for details"
            icon.source: "../images/zoom_in_16px.png"
            background: Rectangle {
                color: "#EFEFEF"
                radius: height / 5
            }
            opacity: control.opacity
            onClicked: {
                if (root.state === "ShowSettings")
                    return
                root.settingsstate = "Show" + smallgraph.state
                root.state = "Show" + smallgraph.state
            }
        }
    }

    states: [
        State {
            name: "Area2D"
            PropertyChanges {
                area2d.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Power output over time"
                header.desctext: "Time-series trends"
                header.imagesource: "../images/meter_48px.png"
            }
        },
        State {
            name: "Bar2D"
            PropertyChanges {
                bar2d.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Energy production per month"
                header.desctext: "Compare discrete categories"
                header.imagesource: "../images/energy-production-icon_48px.png"
            }
        },
        State {
            name: "Surface3D"
            PropertyChanges {
                surface3d.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Power curve visualization"
                header.desctext: "Environmental conditions x efficiency"
                header.imagesource: "../images/power-curve-icon_48px.png"
            }
        },
        State {
            name: "Scatter3D"
            PropertyChanges {
                scatter3d.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Fleet-level performance distribution"
                header.desctext: "Compare performances / patter between turbines"
                header.imagesource: "../images/fleet-level-icon_48px.png"
            }
        },
        State {
            name: "Pie2D"
            PropertyChanges {
                pie2d.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Component failure distribution"
                header.desctext: "Proportional breakdowns"
                header.imagesource: "../images/component-failure-icon_48px.png"
            }
        },
        State {
            name: "Bar3D"
            PropertyChanges {
                bar3d.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Energy production breakdown x Turbine"
                header.desctext: "Compare energy output across turbines"
                header.imagesource: "../images/energy-production-breakdown-icon_48px.png"
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties: "opacity"
                easing.type: Easing.InOutCubic
                duration: 1000
            }
        }
    ]
}
