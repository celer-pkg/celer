// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtGraphs
import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import AerospaceHubExample

Rectangle {
    id: biggraph

    property var activegraph
    property PieSimulator piesimulator
    property BarSimulator barsimulator
    property LineSimulator linesimulator
    property bool detailsview: false

    onActivegraphChanged: {
        GlobalSettings.activegraph = activegraph
        GlobalSettings.is3dgraph = (activegraph === bar3dbig
                                    || activegraph === scatter3dbig
                                    || activegraph === surface3dbig)
    }

    onOpacityChanged: {
        if (detailsview && opacity)
            state = root.state
    }

    state: "ShowArea2D"

    color: "#55000000"
    border.color: "#EFEFEF"
    border.width: 1
    opacity: 0
    visible: opacity > 0.5

    ColumnLayout {
        anchors.fill: parent
        anchors.bottomMargin: GlobalSettings.defaultmargin

        // Header
        GraphHeaderItem {
            id: header
            Layout.fillWidth: true
            Layout.topMargin: GlobalSettings.defaultmargin
            Layout.leftMargin: GlobalSettings.graphdetailmargin
            Layout.preferredHeight: GlobalSettings.graphdetailheaderheight
            titlesize: GlobalSettings.fontsize36px
            descsize: GlobalSettings.fontsize28px
        }

        //! [graphs]
        // Graphs
        Area2DGraph {
            id: area2dbig
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: GlobalSettings.defaultmargin
            opacity: 0
            panStyle: GraphsView.PanStyle.Drag
            zoomStyle: GraphsView.ZoomStyle.Center
            linesimulator: biggraph.linesimulator
            onVisibleChanged: {
                if (visible)
                    biggraph.activegraph = area2dbig
            }
        }

        Bar2DGraph {
            id: bar2dbig
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: GlobalSettings.defaultmargin
            opacity: 0
            panStyle: GraphsView.PanStyle.Drag
            zoomStyle: GraphsView.ZoomStyle.Center
            barsimulator: biggraph.barsimulator
            onVisibleChanged: {
                if (visible)
                    biggraph.activegraph = bar2dbig
            }
        }
        //! [graphs]

        Pie2DGraph {
            id: pie2dbig
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: GlobalSettings.defaultmargin
            opacity: 0
            piesimulator: biggraph.piesimulator
            onVisibleChanged: {
                if (visible)
                    biggraph.activegraph = pie2dbig
            }
        }

        Bar3DGraph {
            id: bar3dbig
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: GlobalSettings.defaultmargin
            opacity: 0
            cameraZoomLevel: GlobalSettings.defaultzoom
            cameraPreset: GlobalSettings.defaultpreset
            rotationEnabled: true
            zoomEnabled: true
            onVisibleChanged: {
                if (visible)
                    biggraph.activegraph = bar3dbig
            }
        }

        Scatter3DGraph {
            id: scatter3dbig
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: GlobalSettings.defaultmargin
            opacity: 0
            cameraZoomLevel: GlobalSettings.defaultzoom
            cameraPreset: GlobalSettings.defaultpreset
            rotationEnabled: true
            zoomEnabled: true
            onVisibleChanged: {
                if (visible)
                    biggraph.activegraph = scatter3dbig
            }
        }

        Surface3DGraph {
            id: surface3dbig
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: GlobalSettings.defaultmargin
            opacity: 0
            cameraZoomLevel: GlobalSettings.defaultzoom
            cameraPreset: GlobalSettings.defaultpreset
            rotationEnabled: true
            zoomEnabled: true
            onVisibleChanged: {
                if (visible)
                    biggraph.activegraph = surface3dbig
            }
        }

        // Control buttons
        RowLayout {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.preferredHeight: GlobalSettings.graphdetailfooterheight
            spacing: GlobalSettings.defaultspacing
            // Panning and zooming don't make sense with pie, hide the button row for it
            opacity: biggraph.activegraph === pie2dbig ? 0 : 1

            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                }
            }

            Button {
                icon.source: GlobalSettings.is3dgraph ? "../images/rotate-icon.png"
                                                      : "../images/pan-icon.png"
                icon.color: "#EFEFEF"
                background: Rectangle {
                    anchors.centerIn: parent
                    width: parent.width
                    height: width
                    radius: width / 8
                    color: parent.pressed ? "#EFEFEF" : "#80181818"
                    border.width: parent.hovered ? 1 : 0
                    border.color: "#EFEFEF"
                    Behavior on color {
                        ColorAnimation {
                            duration: 250
                        }
                    }
                }
                onClicked: {
                    if (GlobalSettings.is3dgraph) {
                        biggraph.activegraph.cameraPreset = GlobalSettings.defaultpreset
                    } else {
                        biggraph.activegraph.axisY.pan = 0
                        if (biggraph.activegraph.axisX.pan)
                            biggraph.activegraph.axisX.pan = 0
                    }
                }
                onHoveredChanged: {
                    tooltip.visible = hovered && biggraph.activegraph !== pie2dbig
                    tooltip.text = GlobalSettings.is3dgraph ? "Reset rotation" : "Reset panning"
                    tooltip.x = x + parent.x + GlobalSettings.doublespacing
                    tooltip.y = parent.y - height / 2
                }
            }

            Button {
                icon.source: "../images/zoom-in-icon.png"
                icon.color: "#EFEFEF"
                background: Rectangle {
                    anchors.centerIn: parent
                    width: parent.width
                    height: width
                    radius: width / 8
                    color: parent.pressed ? "#EFEFEF" : "#80181818"
                    border.width: parent.hovered ? 1 : 0
                    border.color: "#EFEFEF"
                    Behavior on color {
                        ColorAnimation {
                            duration: 250
                        }
                    }
                }
                onClicked: {
                    if (GlobalSettings.is3dgraph) {
                        biggraph.activegraph.cameraZoomLevel = GlobalSettings.defaultzoom
                    } else {
                        biggraph.activegraph.axisY.zoom = 1
                        if (biggraph.activegraph.axisX.zoom)
                            biggraph.activegraph.axisX.zoom = 1
                    }
                }
                onHoveredChanged: {
                    tooltip.visible = hovered && biggraph.activegraph !== pie2dbig
                    tooltip.text = "Reset zoom"
                    tooltip.x = x + parent.x + GlobalSettings.doublespacing
                    tooltip.y = parent.y - height / 2
                }
            }
        }
    }

    ToolTip {
        id: tooltip

        contentItem: Label {
            Layout.fillWidth: true
            color: "#EFEFEF"
            text: tooltip.text
            font.pixelSize: GlobalSettings.fontsize14px
        }
        background: Rectangle {
            color: "#80181818"
            radius: GlobalSettings.quarterradius
            border.width: 1
            border.color: "#80EFEFEF"
        }
    }

    ComboBox {
        id: combobox
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: GlobalSettings.graphdetailmargin
        anchors.topMargin: GlobalSettings.settingscomboboxmargin
        width: GlobalSettings.graphdetailcomboboxwidth

        onCurrentIndexChanged: {
            if (!biggraph.activegraph || !biggraph.activegraph.visible)
                return

            switch (currentIndex) {
            case 0:
                biggraph.state = "ShowArea2D"
                break
            case 1:
                biggraph.state = "ShowBar2D"
                break
            case 2:
                biggraph.state = "ShowPie2D"
                break
            case 3:
                biggraph.state = "ShowBar3D"
                break
            case 4:
                biggraph.state = "ShowScatter3D"
                break
            case 5:
                biggraph.state = "ShowSurface3D"
                break
            }
        }

        model: [
            "2D Area Graph",
            "2D Bar Graph",
            "2D Pie Graph",
            "3D Bar Graph",
            "3D Scatter Graph",
            "3D Surface Graph"
        ]

        background: Rectangle {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: GlobalSettings.comboboxheight
            radius: height / 5
            color: parent.hovered || parent.popup.visible ? "#80181818" : "transparent"
            border.width: 1
            border.color: "#EFEFEF"
        }

        contentItem: Text {
            text: combobox.displayText
            color: "#EFEFEF"
            font.pixelSize: GlobalSettings.fontsize18px
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            leftPadding: GlobalSettings.defaultspacing
        }

        indicator: Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: GlobalSettings.defaultspacing
            source: combobox.popup.visible ? "../images/arrow-head-up-s.png"
                                           : "../images/arrow-head-down-s.png"
        }

        popup: Popup {
            y: parent.height * 2
            width: parent.width
            implicitHeight: contentItem.implicitHeight + header.height / 4

            enter: Transition {
                NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
            }

            exit: Transition {
                NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
            }

            background: Rectangle {
                radius: GlobalSettings.quarterradius
                color: "#80181818"
                border.color: "#EFEFEF"
            }

            contentItem: ListView {
                clip: true
                model: combobox.delegateModel
                implicitHeight: contentHeight
                currentIndex: combobox.highlightedIndex

                delegate: ItemDelegate {
                    required property string modelData
                    width: combobox.width - GlobalSettings.defaultspacing
                    height: GlobalSettings.fontsize28px
                    background: Rectangle {
                        color: parent.hovered ? "#80EFEFEF" : "transparent"
                    }
                    contentItem: Text {
                        text: parent.modelData
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize18px
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "ShowDashboard"
        },
        State {
            name: "ShowArea2D"
            PropertyChanges {
                area2dbig.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Power output over time"
                header.desctext: "Time-series trends"
                header.imagesource: "../images/meter_96px.png"
            }
            PropertyChanges {
                combobox.currentIndex: 0
            }
        },
        State {
            name: "ShowBar2D"
            PropertyChanges {
                bar2dbig.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Energy production per month"
                header.desctext: "Compare discrete categories"
                header.imagesource: "../images/energy-production-icon_96px.png"
            }
            PropertyChanges {
                combobox.currentIndex: 1
            }
        },
        State {
            name: "ShowSurface3D"
            PropertyChanges {
                surface3dbig.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Power curve visualization"
                header.desctext: "Environmental conditions x efficiency"
                header.imagesource: "../images/power-curve-icon_96px.png"
            }
            PropertyChanges {
                combobox.currentIndex: 5
            }
        },
        State {
            name: "ShowScatter3D"
            PropertyChanges {
                scatter3dbig.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Fleet-level performance distribution"
                header.desctext: "Compare performances / patter between turbines"
                header.imagesource: "../images/fleet-level-icon_96px.png"
            }
            PropertyChanges {
                combobox.currentIndex: 4
            }
        },
        State {
            name: "ShowPie2D"
            PropertyChanges {
                pie2dbig.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Component failure distribution"
                header.desctext: "Proportional breakdowns"
                header.imagesource: "../images/component-failure-icon_96px.png"
            }
            PropertyChanges {
                combobox.currentIndex: 2
            }
        },
        State {
            name: "ShowBar3D"
            PropertyChanges {
                bar3dbig.opacity: 1
            }
            PropertyChanges {
                header.titletext: "Energy production breakdown x Turbine"
                header.desctext: "Compare energy output across turbines"
                header.imagesource: "../images/energy-production-breakdown-icon_96px.png"
            }
            PropertyChanges {
                combobox.currentIndex: 3
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
