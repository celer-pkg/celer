// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls.Basic

Rectangle {
    id: mainview
    width: 800
    height: 600
    color: "#404040"

    Row {
        id: toolbar
        anchors.top: parent.top
        anchors.margins: 10
        anchors.left: parent.left
        anchors.leftMargin: 60
        spacing: 10

        Row {
            id: seriesToolbar
            spacing: 10
            Button {
                text: "Theme1"
                onClicked: {
                    seriesTheme.theme = GraphsTheme.Theme.QtGreen;
                }
            }
            Button {
                text: "Theme2"
                onClicked: {
                    seriesTheme.theme = GraphsTheme.Theme.QtGreenNeon;
                }
            }

            Button {
                text: "toggle clipPlotArea"
                onClicked: {
                    chartView.clipPlotArea = !chartView.clipPlotArea;
                }
            }
        }
    }

    Rectangle {
        id: background
        anchors.fill: chartView
        color: "#202020"
        border.color: "#606060"
        border.width: 2
        radius: 10
    }

    GraphsView {
        id: chartView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: toolbar.bottom
        anchors.margins: 10

        axisX: ValueAxis {
            id: xAxis
            max: 4
        }
        axisY: ValueAxis {
            id: yAxis
            max: 8
        }

        theme: GraphsTheme {
            id: seriesTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.QtGreen
            borderColors: ["#E08040"]
            borderWidth: 2
        }

        ScatterSeries {
            id: lineSeries1
            name: "First"
            selectable: true
            pointDelegate: Image {
                property bool pointSelected: false
                source: "images/happy_box.png"
                width: pointSelected ? 96 : 64
                height: pointSelected ? 96 : 64
            }

            XYPoint { x: 0; y: 0 }
            XYPoint { x: 1.1; y: 2.1 }
            XYPoint { x: 1.9; y: 3.3 }
            XYPoint { x: 2.1; y: 2.1 }
            XYPoint { x: 2.9; y: 4.9 }
            XYPoint { x: 3.4; y: 3.0 }
            XYPoint { x: 4.0; y: 3.3 }
        }

        ScatterSeries {
            id: scatterSeries2
            name: "Second"
            draggable: true
            selectable: true

            XYPoint { x: 0; y: 6.6 }
            XYPoint { x: 0.6; y: 4.1 }
            XYPoint { x: 1.5; y: 5.3 }
            XYPoint { x: 2.2; y: 7.1 }
            XYPoint { x: 3.3; y: 6.9 }
            XYPoint { x: 3.6; y: 5.0 }
            XYPoint { x: 4.0; y: 5.3 }
        }

        ScatterSeries {
            id: scatterSeries3
            name: "Third"
            XYPoint { x: 0; y: 2.6 }
            XYPoint { x: 0.2; y: 3.1 }
            XYPoint { x: 1.3; y: 6.3 }
            XYPoint { x: 2.4; y: 5.1 }
            XYPoint { x: 3.5; y: 6.9 }
            XYPoint { x: 3.6; y: 5.2 }
            XYPoint { x: 4.0; y: 3.3 }
        }
        onClipPlotAreaChanged: console.log("clipPlotAreaChanged to ", chartView.clipPlotArea);
    }
}
