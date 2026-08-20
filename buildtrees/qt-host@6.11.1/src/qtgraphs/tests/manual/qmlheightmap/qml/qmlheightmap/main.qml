// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtGraphs
import "."

Item {
    id: mainwindow

    width: 1024
    height: 768
    visible: true

    Item {
        id: surfaceview
        width: mainwindow.width
        height: mainwindow.height

        anchors.top: mainwindow.top
        anchors.left: mainwindow.left

        Gradient {
            id: surfaceGradient
            GradientStop { position: 0.0; color: "darkslategray" }
            GradientStop { id: middleGradient; position: 0.50; color: "peru" }
            GradientStop { position: 1.0; color: "red" }
        }

        GraphsTheme {
            id: mainTheme
            theme: GraphsTheme.Theme.OrangeSeries
            colorStyle: GraphsTheme.ColorStyle.RangeGradient
            baseGradients: [surfaceGradient]
        }

        Surface3D {
            id: surfaceGraph
            width: surfaceview.width
            height: surfaceview.height
            theme: mainTheme
            shadowQuality: Graphs3D.ShadowQuality.Medium
            selectionMode: Graphs3D.SelectionFlag.Slice | Graphs3D.SelectionFlag.ItemAndRow
            cameraPreset: Graphs3D.CameraPreset.IsometricLeft
            axisY.min: 0.0
            axisY.max: 500.0
            axisX.segmentCount: 10
            axisX.subSegmentCount: 2
            axisX.labelFormat: "%i"
            axisZ.segmentCount: 10
            axisZ.subSegmentCount: 2
            axisZ.labelFormat: "%i"
            axisY.segmentCount: 5
            axisY.subSegmentCount: 2
            axisY.labelFormat: "%i"
            axisY.title: "Y"
            axisX.title: "X"
            axisZ.title: "Z"

            Surface3DSeries {
                id: heightSeriesRGB8
                drawMode: Surface3DSeries.DrawSurface
                visible: true
                shading: Surface3DSeries.Shading.Smooth

                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/mapRGB8"
                    minYValue: surfaceGraph.axisY.min
                    maxYValue: surfaceGraph.axisY.max
                }
            }

            Surface3DSeries {
                id: heightSeriesRGB16
                drawMode: Surface3DSeries.DrawSurface
                visible: false
                shading: Surface3DSeries.Shading.Smooth

                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/mapRGB16"
                    minYValue: surfaceGraph.axisY.min
                    maxYValue: surfaceGraph.axisY.max
                }
            }

            Surface3DSeries {
                id: heightSeriesGRAY8
                drawMode: Surface3DSeries.DrawSurface
                visible: false
                shading: Surface3DSeries.Shading.Smooth

                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/mapGRAY8"
                    minYValue: surfaceGraph.axisY.min
                    maxYValue: surfaceGraph.axisY.max
                }
            }

            Surface3DSeries {
                id: heightSeriesGRAY16
                drawMode: Surface3DSeries.DrawSurface
                visible: false
                shading: Surface3DSeries.Shading.Smooth

                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/mapGRAY16"
                    minYValue: surfaceGraph.axisY.min
                    maxYValue: surfaceGraph.axisY.max
                }
            }
        }

        RowLayout {
            id: buttonLayout
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            Button {
                id: toggleHeightSeries
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: qsTr("Use 16-bit rgb map")
                onClicked: {
                    if (heightSeriesRGB8.visible === true) {
                        heightSeriesRGB8.visible = false
                        heightSeriesRGB16.visible = true
                        heightSeriesGRAY8.visible = false
                        heightSeriesGRAY16.visible = false
                        text = "Use 8-bit grayscale map"
                    } else if (heightSeriesRGB16.visible === true){
                        heightSeriesRGB8.visible = false
                        heightSeriesRGB16.visible = false
                        heightSeriesGRAY8.visible = true
                        heightSeriesGRAY16.visible = false
                        text = "Use 16-bit grayscale map"
                    } else if (heightSeriesGRAY8.visible === true){
                        heightSeriesRGB8.visible = false
                        heightSeriesRGB16.visible = false
                        heightSeriesGRAY8.visible = false
                        heightSeriesGRAY16.visible = true
                        text = "Use 8-bit rgb map"
                    } else if (heightSeriesGRAY16.visible === true){
                        heightSeriesRGB8.visible = true
                        heightSeriesRGB16.visible = false
                        heightSeriesGRAY8.visible = false
                        heightSeriesGRAY16.visible = false
                        text = "Use 16-bit rgb map"
                    }
                }
            }

            Button {
                id: toggleAutoScaleY
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: qsTr("Enable autoScaleY")
                onClicked: {
                    if (text === "Enable autoScaleY") {
                        heightSeriesRGB8.dataProxy.autoScaleY = true
                        heightSeriesRGB16.dataProxy.autoScaleY = true
                        heightSeriesGRAY8.dataProxy.autoScaleY = true
                        heightSeriesGRAY16.dataProxy.autoScaleY = true
                        text = "Disable autoScaleY"
                    } else {
                        heightSeriesRGB8.dataProxy.autoScaleY = false
                        heightSeriesRGB16.dataProxy.autoScaleY = false
                        heightSeriesGRAY8.dataProxy.autoScaleY = false
                        heightSeriesGRAY16.dataProxy.autoScaleY = false
                        text = "Enable autoScaleY"
                    }
                }
            }
        }
    }
}
