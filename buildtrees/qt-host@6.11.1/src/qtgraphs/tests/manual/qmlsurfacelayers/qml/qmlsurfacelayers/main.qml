// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtGraphs
import "."

Item {
    id: mainview
    width: 1920
    height: 1080

    property real fontSize: 12
    property real windowRatio: 4
    property real maxAxisSegmentCount: 20

    Item {
        id: surfaceView
        width: mainview.width - buttonLayout.width
        height: mainview.height
        anchors.right: mainview.right;

        //! [0]
        Gradient {
            id: layerOneGradient
            GradientStop { position: 0.0; color: "black" }
            GradientStop { position: 0.31; color: "tan" }
            GradientStop { position: 0.32; color: "green" }
            GradientStop { position: 0.40; color: "darkslategray" }
            GradientStop { position: 1.0; color: "white" }
        }

        Gradient {
            id: layerTwoGradient
            GradientStop { position: 0.315; color: "blue" }
            GradientStop { position: 0.33; color: "white" }
        }

        Gradient {
            id: layerThreeGradient
            GradientStop { position: 0.0; color: "red" }
            GradientStop { position: 0.15; color: "black" }
        }
        //! [0]

        Surface3D {
            id: surfaceLayers
            width: surfaceView.width
            height: surfaceView.height
            theme: GraphsTheme {
                theme: GraphsTheme.Theme.QtGreen
                labelFont.pointSize: 35
                colorStyle: GraphsTheme.ColorStyle.RangeGradient
            }
            shadowQuality: Graphs3D.ShadowQuality.None
            selectionMode: Graphs3D.SelectionFlag.Row | Graphs3D.SelectionFlag.Slice
            cameraPreset: Graphs3D.CameraPreset.IsometricLeft
            axisY.min: 20
            axisY.max: 200
            axisX.segmentCount: 5
            axisX.subSegmentCount: 2
            axisX.labelFormat: "%i"
            axisZ.segmentCount: 5
            axisZ.subSegmentCount: 2
            axisZ.labelFormat: "%i"
            axisY.segmentCount: 5
            axisY.subSegmentCount: 2
            axisY.labelFormat: "%i"

            scene.primarySubViewport: Qt.rect(primaryViewRect.x * windowRatio,
                                                  primaryViewRect.y * windowRatio,
                                                  primaryViewRect.width * windowRatio,
                                                  primaryViewRect.height * windowRatio)

            scene.secondarySubViewport: Qt.rect(secondaryViewRect.x * windowRatio,
                                                secondaryViewRect.y * windowRatio,
                                                secondaryViewRect.width * windowRatio,
                                                secondaryViewRect.height * windowRatio)

            scene.secondarySubviewOnTop: secondaryOnTop.checked

            //! [1]
            //! [2]
            Surface3DSeries {
                id: layerOneSeries
                baseGradient: layerOneGradient
                //! [2]
                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/heightmaps/layer_1.png"
                }
                shading: Surface3DSeries.Shading.Smooth
                drawMode: Surface3DSeries.DrawSurface
                //! [4]
                visible: layerOneToggle.checked // bind to checkbox state
                //! [4]
            }

            Surface3DSeries {
                id: layerTwoSeries
                baseGradient: layerTwoGradient
                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/heightmaps/layer_2.png"
                }
                shading: Surface3DSeries.Shading.Smooth
                drawMode: Surface3DSeries.DrawSurface
                visible: layerTwoToggle.checked // bind to checkbox state
            }

            Surface3DSeries {
                id: layerThreeSeries
                baseGradient: layerThreeGradient
                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/heightmaps/layer_3.png"
                }
                shading: Surface3DSeries.Shading.Smooth
                drawMode: Surface3DSeries.DrawSurface
                visible: layerThreeToggle.checked // bind to checkbox state
            }
            //! [1]
        }
    }

    ColumnLayout {
        id: buttonLayout
        anchors.top: parent.top
        anchors.left: parent.left
        spacing: 0

        //! [3]
        GroupBox {
            Layout.fillWidth: true
            background: Rectangle {
                anchors.fill: parent
                color: "white"
            }

            Column {
                padding: 10
                spacing: 10

                Label {
                    font.pointSize: fontSize
                    font.bold: true
                    text: "Layer Selection"
                }

                CheckBox {
                    id: layerOneToggle
                    checked: true
                    text: "Show Ground Layer"
                }

                CheckBox {
                    id: layerTwoToggle
                    checked: true
                    text: "Show Sea Layer"
                }

                CheckBox {
                    id: layerThreeToggle
                    checked: true
                    text: "Show Tectonic Layer"
                }
            }
        }
        //! [3]

        //! [5]
        GroupBox {
            Layout.fillWidth: true
            background: Rectangle {
                anchors.fill: parent
                color: "white"
            }

            Column {
                padding: 10
                spacing: 10
                Label {
                    font.pointSize: fontSize
                    font.bold: true
                    text: "Layer Style"
                }

                CheckBox {
                    id: layerOneGrid
                    text: "Show Ground as Grid"
                    onCheckedChanged: {
                        if (checked)
                            layerOneSeries.drawMode = Surface3DSeries.DrawWireframe
                        else
                            layerOneSeries.drawMode = Surface3DSeries.DrawSurface
                    }
                }

                CheckBox {
                    id: layerTwoGrid
                    text: "Show Sea as Grid"

                    onCheckedChanged: {
                        if (checked)
                            layerTwoSeries.drawMode = Surface3DSeries.DrawWireframe
                        else
                            layerTwoSeries.drawMode = Surface3DSeries.DrawSurface
                    }
                }

                CheckBox {
                    id: layerThreeGrid
                    text: "Show Tectonic as Grid"
                    onCheckedChanged: {
                        if (checked)
                            layerThreeSeries.drawMode = Surface3DSeries.DrawWireframe
                        else
                            layerThreeSeries.drawMode = Surface3DSeries.DrawSurface
                    }
                }
            }
        }
        //! [5]

        GroupBox {
            Layout.fillWidth: true
            background: Rectangle {
                anchors.fill: parent
                color: "white"
            }

            Column {
                padding :10
                spacing: 10
                Label {
                    font.pointSize: fontSize
                    font.bold: true
                    text: "Axis Segments"
                }

                Column {
                    Label {
                        text: "X Axis Segments"
                    }
                    Slider {
                        id: axisSegmentX
                        from: 1
                        to: maxAxisSegmentCount
                        value: surfaceLayers.axisX.segmentCount

                        onValueChanged: surfaceLayers.axisX.segmentCount = value
                    }

                    Slider {
                        id: subAxisSegmentX
                        from: 1
                        to: maxAxisSegmentCount
                        value: surfaceLayers.axisX.subSegmentCount

                        onValueChanged: surfaceLayers.axisX.subSegmentCount = value
                    }
                }

                Column {
                    Label {
                        text: "Y Axis Segments"
                    }
                    Slider {
                        id: axisSegmentY
                        from: 1
                        to: maxAxisSegmentCount
                        value: surfaceLayers.axisY.segmentCount

                        onValueChanged: surfaceLayers.axisY.segmentCount = value
                    }

                    Slider {
                        id: subAxisSegmentY
                        from: 1
                        to: maxAxisSegmentCount
                        value: surfaceLayers.axisY.subSegmentCount

                        onValueChanged: surfaceLayers.axisY.subSegmentCount = value
                    }
                }

                Column {
                    Label {
                        text: "Z Axis Segments"
                    }
                    Slider {
                        id: axisSegmentZ
                        from: 1
                        to: maxAxisSegmentCount
                        value: surfaceLayers.axisZ.segmentCount

                        onValueChanged: surfaceLayers.axisZ.segmentCount = value
                    }

                    Slider {
                        id: subAxisSegmentZ
                        from: 1
                        to: maxAxisSegmentCount
                        value: surfaceLayers.axisZ.subSegmentCount

                        onValueChanged: surfaceLayers.axisZ.subSegmentCount = value
                    }
                }
            }
        }

        //! [6]
        Button {
            id: sliceButton
            text: "Slice All Layers"
            Layout.fillWidth: true
            Layout.minimumHeight: 40
            onClicked: {
                if (surfaceLayers.selectionMode & Graphs3D.SelectionFlag.MultiSeries) {
                    surfaceLayers.selectionMode = Graphs3D.SelectionFlag.Row
                            | Graphs3D.SelectionFlag.Slice
                    text = "Slice All Layers"
                } else {
                    surfaceLayers.selectionMode = Graphs3D.SelectionFlag.Row
                            | Graphs3D.SelectionFlag.Slice
                            | Graphs3D.SelectionFlag.MultiSeries
                    text = "Slice One Layer"
                }
            }
        }
        //! [6]

        Button {
            id: shadowButton
            Layout.fillWidth: true
            Layout.minimumHeight: 40
            text: "Show Shadows"
            onClicked: {
                if (surfaceLayers.shadowQuality === Graphs3D.ShadowQuality.None) {
                    surfaceLayers.shadowQuality = Graphs3D.ShadowQuality.Low
                    text = "Hide Shadows"
                } else {
                    surfaceLayers.shadowQuality = Graphs3D.ShadowQuality.None
                    text = "Show Shadows"
                }
            }
        }

        Button {
            id: renderModeButton
            text: "Switch Render Mode"
            Layout.fillWidth: true
            Layout.minimumHeight: 40
            onClicked: {
                var modeText = "Indirect "
                var aaText
                if (surfaceLayers.renderingMode === Graphs3D.RenderingMode.Indirect &&
                        surfaceLayers.msaaSamples === 0) {
                    surfaceLayers.renderingMode = Graphs3D.RenderingMode.DirectToBackground
                    modeText = "BackGround "
                } else if (surfaceLayers.renderingMode === Graphs3D.RenderingMode.Indirect &&
                           surfaceLayers.msaaSamples === 4) {
                    surfaceLayers.renderingMode = Graphs3D.RenderingMode.Indirect
                    surfaceLayers.msaaSamples = 0
                } else if (surfaceLayers.renderingMode === Graphs3D.RenderingMode.Indirect &&
                           surfaceLayers.msaaSamples === 8) {
                    surfaceLayers.renderingMode = Graphs3D.RenderingMode.Indirect
                    surfaceLayers.msaaSamples = 4
                } else {
                    surfaceLayers.renderingMode = Graphs3D.RenderingMode.Indirect
                    surfaceLayers.msaaSamples = 8
                }

                if (surfaceLayers.msaaSamples <= 0) {
                    aaText = "No AA"
                } else {
                    aaText = surfaceLayers.msaaSamples + "xMSAA"
                }

                renderLabel.text = modeText + aaText
            }
        }

        TextField {
            id: renderLabel
            font.pointSize: fontSize
            Layout.fillWidth: true
            Layout.minimumHeight: 40
            color: "gray"
            enabled: false
            horizontalAlignment: TextInput.AlignHCenter
            text: "Indirect, " + surfaceLayers.msaaSamples + "xMSAA"
        }
    }

    Rectangle {
        anchors.fill: subviewProps
        anchors.margins: -10
        anchors.rightMargin: -3
    }

    ColumnLayout {
        id: subviewProps
        anchors.bottom: sliceView.visible? sliceView.top : parent.bottom
        anchors.left: parent.left
        anchors.margins: 10

        RowLayout {
            CheckBox {
                id: customSubview
            }
            Label {
                color: "black"
                text: "Customize subviews"
                font.bold: true
            }
        }

        RowLayout {
            visible: customSubview.checked
            CheckBox {
                id: subviewOutline
            }
            Label {
                color: "black"
                text: "Show viewport outline"
            }
        }

        RowLayout {
            visible: customSubview.checked
            CheckBox {
                id: secondaryOnTop
                checked: true
            }
            Label {
                color: "black"
                text: "Secondary view on top"
            }
        }

        RowLayout {
            visible: customSubview.checked
            Rectangle {
                color: "steelblue"
                radius: 3
                height: 10
                width: 10
            }

            Label {
                text: "Primary subview"
                color: "black"
            }
        }
        RowLayout {
            visible: customSubview.checked
            Rectangle {
                color: "aquamarine"
                radius: 3
                height: 10
                width: 10
            }

            Label {
                text: "Secondary subview"
                color: "black"
            }
        }
    }


    Rectangle {
        id: sliceView
        visible: customSubview.checked
        border.color: "lightgrey"
        color: "transparent"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: surfaceView.width / windowRatio
        height: surfaceView.height / windowRatio


        ResizableRect {
            id: primaryViewRect
            rectangleColor: "steelblue"
            width: parent.width / 5
            height: parent.height / 5
        }

        ResizableRect {
            id: secondaryViewRect
            rectangleColor: "aquamarine"
            width: parent.width
            height: parent.height
        }
    }

    Item {
        id: vizBoundary
        anchors.fill: surfaceView
        Rectangle {
            id: primaryViz
            color: "transparent"
            border.color: "steelblue"
            visible: surfaceLayers.scene.slicingActive && subviewOutline.checked
            x: surfaceLayers.scene.primarySubViewport.x
            y: surfaceLayers.scene.primarySubViewport.y
            width: surfaceLayers.scene.primarySubViewport.width
            height: surfaceLayers.scene.primarySubViewport.height
        }

        Rectangle {
            id: secondaryViz
            color: "transparent"
            border.color: "aquamarine"
            visible: surfaceLayers.scene.slicingActive && subviewOutline.checked
            x: surfaceLayers.scene.secondarySubViewport.x
            y: surfaceLayers.scene.secondarySubViewport.y
            width: surfaceLayers.scene.secondarySubViewport.width
            height: surfaceLayers.scene.secondarySubViewport.height
        }
    }

}
