// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls.Basic
import QtQuick.Dialogs

Rectangle {
    id: mainview
    width: 800
    height: 600
    color: "#404040"

    Row {
        anchors.top: parent.top
        anchors.margins: 10
        anchors.left: parent.left
        anchors.leftMargin: 60
        spacing: 10

        id: graphToolbar
        Button {
            text: "Toggle axisX"
            onClicked: {
                if (!chartView.axisX)
                    chartView.axisX = xAxis
                else
                    chartView.axisX = null
            }
        }
        Button {
            text: "Toggle axisY"
            onClicked: {
                if (!chartView.axisY)
                    chartView.axisY = yAxis
                else
                    chartView.axisY = null
            }
        }
        Button {
            text: "axisX alignment"
            onClicked: {
                if (xAxis.alignment != Qt.AlignTop)
                    xAxis.alignment = Qt.AlignTop
                else
                    xAxis.alignment = Qt.AlignBottom
            }
        }
        Button {
            text: "axisY alignment"
            onClicked: {
                if (yAxis.alignment != Qt.AlignRight)
                    yAxis.alignment = Qt.AlignRight
                else
                    yAxis.alignment = Qt.AlignLeft
            }
        }
        Button {
            text: "Disable multi axis"
            onClicked: {
                barSeries.axisX = null
                scatterSeries.axisX = null
                lineSeries.axisX = null
                splineSeries.axisX = null
                areaSeries.axisX = null
                barSeries.axisY = null
                scatterSeries.axisY = null
                lineSeries.axisY = null
                splineSeries.axisY = null
                areaSeries.axisY = null
            }
        }
        Button {
            text: "Change orientation"
            onClicked: {
                if (chartView.orientation != Qt.Horizontal)
                    chartView.orientation = Qt.Horizontal
                else
                    chartView.orientation = Qt.Vertical
            }
        }
        Button {
            text: "Set axis custom colors"
            onClicked: {
                yAxis.subColor = "red"
                xAxis.color = "salmon"
                xAxis2.subColor = "blue"
                yAxis2.color = "orange"
            }
        }
        Button {
            text: "Reset axis theme colors"
            onClicked: {
                yAxis.subColor = null
                xAxis.color = null
                xAxis2.subColor = null
                yAxis2.color = null
            }
        }
        Button {
            text: "Toggle titles"
            onClicked: {
                if (xAxis.titleText === "") {
                    xAxis.titleText = "Years"
                    yAxis.titleText = "Numbers"
                    lineSeries.axisX.titleText = lineSeries.name
                    splineSeries.axisX.titleText = splineSeries.name
                    scatterSeries.axisY.titleText = scatterSeries.name
                    areaSeries.axisY.titleText = areaSeries.name
                } else {
                    xAxis.titleText = ""
                    yAxis.titleText = ""
                    lineSeries.axisX.titleText = ""
                    splineSeries.axisX.titleText = ""
                    scatterSeries.axisY.titleText = ""
                    areaSeries.axisY.titleText = ""
                }
            }
        }
    }

    GraphsView {
        id: chartView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: graphToolbar.bottom
        anchors.margins: 10

        axisX: BarCategoryAxis {
            id: xAxis
            categories: ["2023", "2024", "2025", "2026"]
        }
        axisY: ValueAxis {
            id: yAxis
            max: 8
            subTickCount: 4
        }

        onHoverEnter: {
            tooltip.visible = true;
        }

        onHoverExit: {
            tooltip.visible = false;
        }

        onHover: (seriesName, position, value) => {
            tooltip.x = position.x + 1;
            tooltip.y = position.y + 1;
            tooltip.text = "Series: " + seriesName + ", X: " + value.x.toFixed(1) + ", Y: " + value.y.toFixed(1);
        }

        BarSeries {
            id: barSeries
            name: "Bars"
            hoverable: true
            seriesColors: ["#d0d040", "#d04040"]
            borderColors: ["#808020", "#802020"]
            BarSet { id: set1; label: "Axel"; values: [1, 2, 3, 4] }
            BarSet { id: set2; label: "Bob"; values: [4, 3, 2, 1] }
        }

        LineSeries {
            id: lineSeries
            name: "Lines"
            hoverable: true
            axisX: ValueAxis {
                id: xAxis2
                max: 8
                subTickCount: 2
            }

            XYPoint { x: 0; y: 6.6 }
            XYPoint { x: 0.6; y: 4.1 }
            XYPoint { x: 1.5; y: 5.3 }
            XYPoint { x: 2.2; y: 7.1 }
            XYPoint { x: 3.3; y: 6.9 }
            XYPoint { x: 3.6; y: 5.0 }
            XYPoint { x: 4.0; y: 5.3 }
        }

        SplineSeries {
            id: splineSeries
            name: "Spline"
            hoverable: true
            axisX: ValueAxis {
                max: 100
                alignment: Qt.AlignTop
            }

            XYPoint { x: 0; y: 4.6 }
            XYPoint { x: 0.6; y: 2.1 }
            XYPoint { x: 1.5; y: 3.3 }
            XYPoint { x: 2.2; y: 5.1 }
            XYPoint { x: 3.3; y: 4.9 }
            XYPoint { x: 3.6; y: 3.0 }
        }

        ScatterSeries {
            id: scatterSeries
            name: "Points"
            hoverable: true
            axisY: ValueAxis {
                max: 12
            }

            XYPoint { x: 0; y: 2.6 }
            XYPoint { x: 0.2; y: 3.1 }
            XYPoint { x: 1.3; y: 8.3 }
            XYPoint { x: 2.4; y: 9.1 }
            XYPoint { x: 3.5; y: 10.9 }
            XYPoint { x: 3.6; y: 5.2 }
            XYPoint { x: 4.0; y: 3.3 }
        }

        AreaSeries {
            id: areaSeries
            name: "Batman"
            hoverable: true
            axisY: ValueAxis {
                id: yAxis2
                max: 10
                alignment: Qt.AlignRight
            }

            upperSeries: LineSeries {
                XYPoint { x: 1; y: 5 }
                XYPoint { x: 1.25; y: 6 }
                XYPoint { x: 1.75; y: 5.5 }
                XYPoint { x: 2; y: 6 }
                XYPoint { x: 2.375; y: 5.5 }
                XYPoint { x: 2.875; y: 6 }
                XYPoint { x: 3.375; y: 5 }
            }

            lowerSeries: LineSeries {
                XYPoint { x: 1; y: 4 }
                XYPoint { x: 1.25; y: 4.5 }
                XYPoint { x: 1.75; y: 4 }
                XYPoint { x: 1.875; y: 3.5 }
                XYPoint { x: 2.375; y: 4 }
                XYPoint { x: 3; y: 4.5 }
                XYPoint { x: 3.375; y: 4 }
            }
        }

        ToolTip {
            id: tooltip
        }
    }
}
