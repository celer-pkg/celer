// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtDataVisualization
import AxisHandling

Item {
    id: axisFormattingView

    required property bool portraitMode

    Data {
        id: seriesData
    }

    Theme3D {
        id: themePrimaryColors
        type: Theme3D.ThemePrimaryColors
        font.family: "Lucida Handwriting"
        font.pointSize: 40
    }

    //! [1]
    ValueAxis3D {
        id: dateAxis
        formatter: CustomFormatter {
            originDate: "2023-01-01"
            selectionFormat: "yyyy-MM-dd HH:mm:ss"
        }
        subSegmentCount: 2
        labelFormat: "yyyy-MM-dd"
        min: 0
        max: 14
    }
    //! [1]

    //! [2]
    ValueAxis3D {
        id: logAxis
        formatter: LogValueAxis3DFormatter {
            id: logAxisFormatter
            base: 10
            autoSubGrid: true
            showEdgeLabels: true
        }
        labelFormat: "%.2f"
    }
    //! [2]

    ValueAxis3D {
        id: linearAxis
        labelFormat: "%.2f"
        min: 0
        max: 500
    }

    //! [0]
    ValueAxis3D {
        id: valueAxis
        segmentCount: 5
        subSegmentCount: 2
        labelFormat: "%.2f"
        min: 0
        max: 10
    }
    //! [0]

    Scatter3D {
        id: scatterGraph
        anchors.top: exitButton.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        theme: themePrimaryColors
        shadowQuality: AbstractGraph3D.ShadowQualitySoftMedium
        scene.activeCamera.cameraPreset: Camera3D.CameraPresetIsometricRight
        //! [3]
        axisZ: valueAxis
        axisY: logAxis
        axisX: dateAxis
        //! [3]

        Scatter3DSeries {
            id: scatterSeries
            itemLabelFormat: "@xLabel - (@yLabel, @zLabel)"
            meshSmooth: true
            ItemModelScatterDataProxy {
                itemModel: seriesData.model
                xPosRole: "xPos"
                yPosRole: "yPos"
                zPosRole: "zPos"
            }
        }
    }

    Button {
        id: yAxisBaseChange
        width: axisFormattingView.portraitMode ? parent.width : parent.width / 3
        anchors.left: parent.left
        anchors.top: parent.top
        state: "enabled"
        onClicked: {
            if (logAxisFormatter.base === 10)
                logAxisFormatter.base = 0;
            else if (logAxisFormatter.base === 2)
                logAxisFormatter.base = 10;
            else
                logAxisFormatter.base = 2;
        }
        states: [
            State {
                name: "enabled"
                PropertyChanges {
                    target: yAxisBaseChange
                    text: "Y-axis log base: " + logAxisFormatter.base
                    enabled: true
                }
            },
            State {
                name: "disabled"
                PropertyChanges {
                    target: yAxisBaseChange
                    text: "Y-axis linear"
                    enabled: false
                }
            }
        ]
    }

    Button {
        id: yAxisToggle
        width: axisFormattingView.portraitMode ? parent.width : parent.width / 3
        anchors.left: axisFormattingView.portraitMode ? parent.left : yAxisBaseChange.right
        anchors.top: axisFormattingView.portraitMode ? yAxisBaseChange.bottom : parent.top
        text: "Toggle Y-axis"
        onClicked: {
            if (scatterGraph.axisY == linearAxis) {
                scatterGraph.axisY = logAxis;
                yAxisBaseChange.state = "enabled";
            } else {
                scatterGraph.axisY = linearAxis;
                yAxisBaseChange.state = "disabled";
            }
        }
    }

    Button {
        id: exitButton
        width: axisFormattingView.portraitMode ? parent.width : parent.width / 3
        anchors.left: axisFormattingView.portraitMode ? parent.left : yAxisToggle.right
        anchors.top: axisFormattingView.portraitMode ? yAxisToggle.bottom : parent.top
        text: "Quit"
        onClicked: Qt.quit();
    }
}
