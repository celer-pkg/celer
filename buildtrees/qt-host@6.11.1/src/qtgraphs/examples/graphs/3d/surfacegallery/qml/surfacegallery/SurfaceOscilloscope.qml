// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Basic
import QtGraphs
//! [0]
import SurfaceGalleryExample
//! [0]

Item {
    id: oscilloscopeView

    property int sampleColumns: sampleSlider.value
    property int sampleRows: sampleColumns / 2
    property int sampleCache: 24

    required property bool portraitMode

    property real controlWidth: oscilloscopeView.portraitMode ? oscilloscopeView.width - 10
                                                              : oscilloscopeView.width / 4 - 6.66

    property real buttonWidth: oscilloscopeView.portraitMode ? oscilloscopeView.width - 10
                                                             : oscilloscopeView.width / 3 - 7.5

    onSampleRowsChanged: {
        surfaceSeries.selectedPoint = surfaceSeries.invalidSelectionPosition
        generateData()
    }

    //![1]
    DataSource {
        id: dataSource
    }
    //![1]

    //! [7]
    Timer {
        id: refreshTimer
        interval: 1000 / frequencySlider.value
        running: true
        repeat: true
        onTriggered: dataSource.update(surfaceSeries);
    }
    //! [7]

    Rectangle {
        id: controlArea
        height: oscilloscopeView.portraitMode ? flatShadingToggle.implicitHeight * 8
                                              : flatShadingToggle.implicitHeight * 2.5
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        color: surfaceGraph.theme.backgroundColor

        // Samples
        Rectangle {
            id: samples
            width: oscilloscopeView.controlWidth
            height: flatShadingToggle.implicitHeight
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 5

            color: surfaceGraph.theme.backgroundColor
            border.color: surfaceGraph.theme.grid.mainColor
            border.width: 1
            radius: 4

            Row {
                anchors.centerIn: parent
                spacing: 10
                padding: 5

                Slider {
                    id: sampleSlider
                    from: oscilloscopeView.sampleCache * 2
                    to: from * 10
                    stepSize: oscilloscopeView.sampleCache

                    background: Rectangle {
                        x: sampleSlider.leftPadding
                        y: sampleSlider.topPadding + sampleSlider.availableHeight / 2
                           - height / 2
                        implicitWidth: samples.width > 300 ? 200 : samples.width
                                        - samplesText.width - 20
                        implicitHeight: 4
                        width: sampleSlider.availableWidth
                        height: implicitHeight
                        radius: 2
                        color: surfaceGraph.theme.grid.mainColor

                        Rectangle {
                            width: sampleSlider.visualPosition * parent.width
                            height: parent.height
                            color: surfaceGraph.theme.labelTextColor
                            radius: 2
                        }
                    }

                    handle: Rectangle {
                        x: sampleSlider.leftPadding + sampleSlider.visualPosition
                           * (sampleSlider.availableWidth - width)
                        y: sampleSlider.topPadding + sampleSlider.availableHeight / 2
                           - height / 2
                        implicitWidth: 20
                        implicitHeight: 20
                        radius: 10
                        color: sampleSlider.pressed ? surfaceGraph.theme.grid.mainColor
                                                    : surfaceGraph.theme.backgroundColor
                        border.color: sampleSlider.pressed ? surfaceGraph.theme.labelTextColor
                                                           : surfaceGraph.theme.grid.mainColor
                    }

                    Component.onCompleted: value = from;
                }

                Text {
                    id: samplesText
                    text: "Samples: " + (oscilloscopeView.sampleRows * oscilloscopeView.sampleColumns)
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: surfaceGraph.theme.labelTextColor
                }
            }
        }

        // Frequency
        Rectangle {
            id: frequency
            width: oscilloscopeView.controlWidth
            height: flatShadingToggle.implicitHeight
            anchors.left: oscilloscopeView.portraitMode ? parent.left : samples.right
            anchors.top: oscilloscopeView.portraitMode ? samples.bottom : parent.top
            anchors.margins: 5

            color: surfaceGraph.theme.backgroundColor
            border.color: surfaceGraph.theme.grid.mainColor
            border.width: 1
            radius: 4

            Row {
                anchors.centerIn: parent
                spacing: 10
                padding: 5

                Slider {
                    id: frequencySlider
                    from: 2
                    to: 60
                    stepSize: 2
                    value: 30

                    background: Rectangle {
                        x: frequencySlider.leftPadding
                        y: frequencySlider.topPadding + frequencySlider.availableHeight / 2
                           - height / 2
                        implicitWidth: frequency.width > 300 ? 200 : frequency.width
                                        - frequencyText.width - 20
                        implicitHeight: 4
                        width: frequencySlider.availableWidth
                        height: implicitHeight
                        radius: 2
                        color: surfaceGraph.theme.grid.mainColor

                        Rectangle {
                            width: frequencySlider.visualPosition * parent.width
                            height: parent.height
                            color: surfaceGraph.theme.labelTextColor
                            radius: 2
                        }
                    }

                    handle: Rectangle {
                        x: frequencySlider.leftPadding + frequencySlider.visualPosition
                           * (frequencySlider.availableWidth - width)
                        y: frequencySlider.topPadding + frequencySlider.availableHeight / 2
                           - height / 2
                        implicitWidth: 20
                        implicitHeight: 20
                        radius: 10
                        color: frequencySlider.pressed ? surfaceGraph.theme.grid.mainColor
                                                       : surfaceGraph.theme.backgroundColor
                        border.color: frequencySlider.pressed ? surfaceGraph.theme.labelTextColor
                                                              : surfaceGraph.theme.grid.mainColor
                    }
                }

                Text {
                    id: frequencyText
                    text: "Freq: " + frequencySlider.value + " Hz"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: surfaceGraph.theme.labelTextColor
                }
            }
        }

        // FPS
        Rectangle {
            id: fpsindicator
            width: oscilloscopeView.controlWidth
            height: flatShadingToggle.implicitHeight
            anchors.left: oscilloscopeView.portraitMode ? parent.left : frequency.right
            anchors.top: oscilloscopeView.portraitMode ? frequency.bottom : parent.top
            anchors.margins: 5

            color: surfaceGraph.theme.backgroundColor
            border.color: surfaceGraph.theme.grid.mainColor
            border.width: 1
            radius: 4

            Text {
                id: fpsText
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: surfaceGraph.theme.labelTextColor
            }
        }

        // Selection
        Rectangle {
            id: selection
            width: oscilloscopeView.controlWidth
            height: flatShadingToggle.implicitHeight
            anchors.left: oscilloscopeView.portraitMode ? parent.left : fpsindicator.right
            anchors.top: oscilloscopeView.portraitMode ? fpsindicator.bottom : parent.top
            anchors.margins: 5

            color: surfaceGraph.theme.backgroundColor
            border.color: surfaceGraph.theme.grid.mainColor
            border.width: 1
            radius: 4

            Text {
                id: selectionText
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "No selection"
                color: surfaceGraph.theme.labelTextColor
            }
        }

        // Flat shading
        Button {
            id: flatShadingToggle
            width: oscilloscopeView.buttonWidth
            anchors.left: parent.left
            anchors.top: selection.bottom
            anchors.margins: 5

            text: surfaceSeries.flatShadingSupported ? "Show\nSmooth" : "Flat\nnot supported"
            enabled: surfaceSeries.flatShadingSupported

            onClicked: {
                if (surfaceSeries.shading === Surface3DSeries.Shading.Flat) {
                    surfaceSeries.shading = Surface3DSeries.Shading.Smooth;
                    text = "Show\nFlat"
                } else {
                    surfaceSeries.shading = Surface3DSeries.Shading.Flat;
                    text = "Show\nSmooth"
                }
            }

            contentItem: Text {
                text: flatShadingToggle.text
                opacity: flatShadingToggle.enabled ? 1.0 : 0.3
                color: surfaceGraph.theme.labelTextColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            background: Rectangle {
                opacity: flatShadingToggle.enabled ? 1 : 0.3
                color: flatShadingToggle.down ? surfaceGraph.theme.grid.mainColor
                                              : surfaceGraph.theme.backgroundColor
                border.color: flatShadingToggle.down ? surfaceGraph.theme.labelTextColor
                                                     : surfaceGraph.theme.grid.mainColor
                border.width: 1
                radius: 2
            }
        }

        // Surface grid
        Button {
            id: surfaceGridToggle
            width: oscilloscopeView.buttonWidth
            anchors.left: oscilloscopeView.portraitMode ? parent.left : flatShadingToggle.right
            anchors.top: oscilloscopeView.portraitMode ? flatShadingToggle.bottom : selection.bottom
            anchors.margins: 5

            text: "Hide\nSurface Grid"

            onClicked: {
                if (surfaceSeries.drawMode & Surface3DSeries.DrawWireframe) {
                    surfaceSeries.drawMode &= ~Surface3DSeries.DrawWireframe;
                    text = "Show\nSurface Grid";
                } else {
                    surfaceSeries.drawMode |= Surface3DSeries.DrawWireframe;
                    text = "Hide\nSurface Grid";
                }
            }

            contentItem: Text {
                text: surfaceGridToggle.text
                color: surfaceGraph.theme.labelTextColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            background: Rectangle {
                color: surfaceGridToggle.down ? surfaceGraph.theme.grid.mainColor
                                              : surfaceGraph.theme.backgroundColor
                border.color: surfaceGridToggle.down ? surfaceGraph.theme.labelTextColor
                                                     : surfaceGraph.theme.grid.mainColor
                border.width: 1
                radius: 2
            }
        }

        // Exit
        Button {
            id: exitButton
            width: oscilloscopeView.buttonWidth
            height: surfaceGridToggle.height
            anchors.left: oscilloscopeView.portraitMode ? parent.left : surfaceGridToggle.right
            anchors.top: oscilloscopeView.portraitMode ? surfaceGridToggle.bottom : selection.bottom
            anchors.margins: 5

            text: "Quit"

            onClicked: Qt.quit();

            contentItem: Text {
                text: exitButton.text
                color: surfaceGraph.theme.labelTextColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            background: Rectangle {
                color: exitButton.down ? surfaceGraph.theme.grid.mainColor
                                       : surfaceGraph.theme.backgroundColor
                border.color: exitButton.down ? surfaceGraph.theme.labelTextColor
                                              : surfaceGraph.theme.grid.mainColor
                border.width: 1
                radius: 2
            }
        }
    }

    Item {
        id: dataView
        anchors.top: controlArea.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        //! [2]
        Surface3D {
            id: surfaceGraph
            anchors.fill: parent

            Surface3DSeries {
                id: surfaceSeries
                drawMode: Surface3DSeries.DrawSurfaceAndWireframe
                itemLabelFormat: "@xLabel, @zLabel: @yLabel"
                //! [2]
                //! [3]
                itemLabelVisible: false
                //! [3]

                //! [4]
                onItemLabelChanged: {
                    if (surfaceSeries.selectedPoint == surfaceSeries.invalidSelectionPosition)
                        selectionText.text = "No selection";
                    else
                        selectionText.text = surfaceSeries.itemLabel;
                }
                //! [4]
            }

            shadowQuality: Graphs3D.ShadowQuality.None
            selectionMode: Graphs3D.SelectionFlag.Slice | Graphs3D.SelectionFlag.ItemAndColumn
            theme: GraphsTheme {
                colorScheme: GraphsTheme.ColorScheme.Dark
                baseColors: [ Color { color: "yellow" } ]
                plotAreaBackgroundVisible: false
                backgroundVisible: false
                labelBorderVisible: false
                labelBackgroundVisible: false
            }
            cameraPreset: Graphs3D.CameraPreset.FrontHigh

            axisX.labelFormat: "%d ms"
            axisY.labelFormat: "%d W"
            axisZ.labelFormat: "%d mV"
            axisX.min: 0
            axisY.min: 0
            axisZ.min: 0
            axisX.max: 1000
            axisY.max: 100
            axisZ.max: 800
            axisX.segmentCount: 4
            axisY.segmentCount: 4
            axisZ.segmentCount: 4
            measureFps: true
            renderingMode: Graphs3D.RenderingMode.DirectToBackground

            onCurrentFpsChanged: (currentFps)=> {
                                     fpsText.text = "FPS: " + currentFps;
                                 }

            //! [5]
            Component.onCompleted: oscilloscopeView.generateData();
            //! [5]
        }
    }

    //! [6]
    function generateData() {
        dataSource.generateData(oscilloscopeView.sampleCache, oscilloscopeView.sampleRows,
                                oscilloscopeView.sampleColumns,
                                surfaceGraph.axisX.min, surfaceGraph.axisX.max,
                                surfaceGraph.axisY.min, surfaceGraph.axisY.max,
                                surfaceGraph.axisZ.min, surfaceGraph.axisZ.max);
    }
    //! [6]
}
