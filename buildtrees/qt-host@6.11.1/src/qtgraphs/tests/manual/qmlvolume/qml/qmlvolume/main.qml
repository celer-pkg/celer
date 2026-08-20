// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtGraphs
import "."

Item {
    id: mainView
    width: 1280
    height: 1024

    Item {
        id: dataView
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height - buttonLayout.height

        Surface3D {
            id: surfaceGraph

            width: dataView.width
            height: dataView.height
            orthoProjection: true
            measureFps : false
            Surface3DSeries {
                id: surfaceSeries
                drawMode: Surface3DSeries.DrawSurface;
                shading: Surface3DSeries.Shading.Smooth
                meshSmooth: true
                itemLabelFormat: "@xLabel, @zLabel: @yLabel"
                itemLabelVisible: false

                onItemLabelChanged: {
                    if (surfaceSeries.selectedPoint === surfaceSeries.invalidSelectionPosition)
                        selectionText.text = "No selection"
                    else
                        selectionText.text = surfaceSeries.itemLabel
                }
            }

            Component.onCompleted: {
                mainView.createVolume();
            }
        }
    }

    Rectangle {
        width: parent.width
        height: 50
        anchors.left: parent.left
        anchors.top: parent.top
        color: surfaceGraph.theme.plotAreaBackgroundColor

        ColumnLayout {
            anchors.fill: parent
            RowLayout {
                id: sliderLayout
                Layout.fillHeight: false
                spacing: 0
                visible: surfaceGraph.measureFps
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumWidth: fpsText.implicitWidth + 10
                    Layout.maximumWidth: fpsText.implicitWidth + 10
                    Layout.minimumHeight: 50
                    Layout.maximumHeight: 50
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    border.color: "gray"
                    border.width: 1
                    radius: 4

                    Text {
                        id: fpsText
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: {
                            return "FPS: " + surfaceGraph.currentFps > 10
                                    ? Math.round(surfaceGraph.currentFps)
                                    : Math.round(surfaceGraph.currentFps * 10.0) / 10.0
                        }
                    }
                }
            }

            RowLayout {
                id: buttonLayout
                Layout.minimumHeight: 50
                spacing: 0

                Button {
                    id: sliceButton
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    text: "Slice"

                    onClicked: {
                        if (volumeItem.sliceIndexZ == -1) {
                            volumeItem.sliceIndexZ = 128
                            volumeItem.drawSlices = true
                            volumeItem.drawSliceFrames = true
                        } else {
                            volumeItem.sliceIndexZ = -1
                            volumeItem.drawSlices = false
                            volumeItem.drawSliceFrames = false
                        }
                    }
                }
                Button {
                    id: exitButton
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    text: "Quit"

                    onClicked: Qt.quit();
                }
            }
        }

    }

    Custom3DVolume {
        id: volumeItem
        alphaMultiplier: 0.3
        preserveOpacity: true
        useHighDefShader: false
    }

    function createVolume() {
        surfaceGraph.addCustomItem(volumeItem)
        dataSource.fillVolume(volumeItem)
    }
}
