// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtGraphs

Rectangle {
    id: spectrogramView
    color: surfaceGraph.theme.backgroundColor

    required property bool portraitMode

    property real buttonWidth: spectrogramView.portraitMode ? (spectrogramView.width - 35) / 2
                                                            : (spectrogramView.width - 50) / 5

    SpectrogramData {
        id: surfaceData
    }

    Item {
        id: surfaceView
        anchors.top: buttons.bottom
        anchors.left: parent.left
        anchors.right: legend.left
        anchors.bottom: parent.bottom

        Gradient {
            id: surfaceGradient
            GradientStop { position: 0.0; color: "black" }
            GradientStop { position: 0.2; color: "red" }
            GradientStop { position: 0.5; color: "blue" }
            GradientStop { position: 0.8; color: "yellow" }
            GradientStop { position: 1.0; color: "white" }
        }

        Value3DAxis {
            id: xAxis
            segmentCount: 8
            labelFormat: "%i\u00B0"
            title: "Angle"
            titleVisible: true
            titleFixed: false
        }

        Value3DAxis {
            id: yAxis
            segmentCount: 8
            labelFormat: "%i \%"
            title: "Value"
            titleVisible: true
            labelAutoAngle: 0
            titleFixed: false
        }

        Value3DAxis {
            id: zAxis
            segmentCount: 5
            labelFormat: "%i nm"
            title: "Radius"
            titleVisible: true
            titleFixed: false
        }

        GraphsTheme {
            id: customTheme
            theme: GraphsTheme.Theme.QtGreen
            backgroundVisible: false
            grid.mainColor: "#AAAAAA"
            backgroundColor: "#EEEEEE"
        }

        //! [0]
        Surface3D {
            id: surfaceGraph
            anchors.fill: parent

            // Don't show specular spotlight as we don't want it to distort the colors
            lightStrength: 0.0
            ambientLightStrength: 1.0

            Surface3DSeries {
                id: surfaceSeries
                shading: Surface3DSeries.Shading.Smooth
                drawMode: Surface3DSeries.DrawSurface
                baseGradient: surfaceGradient
                colorStyle: GraphsTheme.ColorStyle.RangeGradient
                itemLabelFormat: "(@xLabel, @zLabel): @yLabel"

                ItemModelSurfaceDataProxy {
                    itemModel: surfaceData.model
                    rowRole: "radius"
                    columnRole: "angle"
                    yPosRole: "value"
                }
            }
            //! [0]

            //! [1]
            // Remove the perspective and view the graph from top down to achieve 2D effect
            orthoProjection: true
            cameraPreset: Graphs3D.CameraPreset.DirectlyAbove
            //! [1]

            //! [2]
            flipHorizontalGrid: true
            //! [2]

            //! [4]
            radialLabelOffset: 0.01
            //! [4]

            //! [5]
            rotationEnabled: !surfaceGraph.orthoProjection
            //! [5]

            theme: customTheme
            shadowQuality: Graphs3D.ShadowQuality.None
            selectionMode: Graphs3D.SelectionFlag.Slice | Graphs3D.SelectionFlag.ItemAndColumn
            axisX: xAxis
            axisY: yAxis
            axisZ: zAxis

            aspectRatio: 1.0
            horizontalAspectRatio: 1.0
            cameraZoomLevel: 140
        }
    }

    Item {
        id: buttons
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: spectrogramView.portraitMode ? (polarToggle.height + 10) * 3
                                             : polarToggle.height + 30
        anchors.margins: 10

        //! [3]
        Button {
            id: polarToggle
            anchors.margins: 5
            anchors.left: parent.left
            anchors.top: parent.top
            width: spectrogramView.buttonWidth // Calculated elsewhere based on screen orientation
            text: "Switch to\n" + (surfaceGraph.polar ? "cartesian" : "polar")
            onClicked: surfaceGraph.polar = !surfaceGraph.polar;
        }
        //! [3]

        Button {
            id: orthoToggle
            anchors.margins: 5
            anchors.left: polarToggle.right
            anchors.top: parent.top
            width: spectrogramView.buttonWidth
            text: "Switch to\n" + (surfaceGraph.orthoProjection ? "perspective" : "orthographic")
            onClicked: {
                if (surfaceGraph.orthoProjection) {
                    surfaceGraph.orthoProjection = false;
                    xAxis.labelAutoAngle = 30;
                    yAxis.labelAutoAngle = 30;
                    zAxis.labelAutoAngle = 30;
                } else {
                    surfaceGraph.orthoProjection = true;
                    surfaceGraph.cameraPreset
                            = Graphs3D.CameraPreset.DirectlyAbove;
                    surfaceSeries.drawMode &= ~Surface3DSeries.DrawWireframe;
                    xAxis.labelAutoAngle = 0;
                    yAxis.labelAutoAngle = 0;
                    zAxis.labelAutoAngle = 0;
                }
            }
        }

        Button {
            id: flipGridToggle
            anchors.margins: 5
            anchors.left: spectrogramView.portraitMode ? parent.left : orthoToggle.right
            anchors.top: spectrogramView.portraitMode ? orthoToggle.bottom : parent.top
            width: spectrogramView.buttonWidth
            text: "Toggle axis\ngrid on top"
            onClicked: surfaceGraph.flipHorizontalGrid = !surfaceGraph.flipHorizontalGrid;
        }

        Button {
            id: labelOffsetToggle
            anchors.margins: 5
            anchors.left: flipGridToggle.right
            anchors.top: spectrogramView.portraitMode ? orthoToggle.bottom : parent.top
            width: spectrogramView.buttonWidth
            text: "Toggle radial\nlabel position"
            visible: surfaceGraph.polar
            onClicked: {
                if (surfaceGraph.radialLabelOffset >= 1.0)
                    surfaceGraph.radialLabelOffset = 0.01;
                else
                    surfaceGraph.radialLabelOffset = 1.0;
            }
        }

        Button {
            id: surfaceGridToggle
            anchors.margins: 5
            anchors.left: spectrogramView.portraitMode ? (labelOffsetToggle.visible ? parent.left
                                                                                    : flipGridToggle.right)
                                                       : (labelOffsetToggle.visible ? labelOffsetToggle.right
                                                                                    : flipGridToggle.right)
            anchors.top: spectrogramView.portraitMode ? (labelOffsetToggle.visible ? labelOffsetToggle.bottom
                                                                                   : orthoToggle.bottom)
                                                      : parent.top
            width: spectrogramView.buttonWidth
            text: "Toggle\nsurface grid"
            visible: !surfaceGraph.orthoProjection
            onClicked: {
                if (surfaceSeries.drawMode & Surface3DSeries.DrawWireframe)
                    surfaceSeries.drawMode &= ~Surface3DSeries.DrawWireframe;
                else
                    surfaceSeries.drawMode |= Surface3DSeries.DrawWireframe;
            }
        }
    }

    Item {
        id: legend
        anchors.bottom: parent.bottom
        anchors.top: buttons.bottom
        anchors.right: parent.right
        width: spectrogramView.portraitMode ? 100 : 125

        Rectangle {
            id: gradient
            anchors.margins: 20
            anchors.bottom: legend.bottom
            anchors.top: legend.top
            anchors.right: legend.right
            border.color: "black"
            border.width: 1
            width: spectrogramView.portraitMode ? 25 : 50
            rotation: 180
            gradient: Gradient {
                GradientStop { position: 0.0; color: "black" }
                GradientStop { position: 0.2; color: "red" }
                GradientStop { position: 0.5; color: "blue" }
                GradientStop { position: 0.8; color: "yellow" }
                GradientStop { position: 1.0; color: "white" }
            }
        }

        Text {
            anchors.verticalCenter: gradient.bottom
            anchors.right: gradient.left
            anchors.margins: 2
            text: surfaceGraph.axisY.min  + "%"
        }

        Text {
            anchors.verticalCenter: gradient.verticalCenter
            anchors.right: gradient.left
            anchors.margins: 2
            text: (surfaceGraph.axisY.max + surfaceGraph.axisY.min) / 2  + "%"
        }

        Text {
            anchors.verticalCenter: gradient.top
            anchors.right: gradient.left
            anchors.margins: 2
            text: surfaceGraph.axisY.max + "%"
        }
    }
}
