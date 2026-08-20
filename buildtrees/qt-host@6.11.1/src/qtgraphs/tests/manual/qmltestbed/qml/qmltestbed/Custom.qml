// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import TestbedExample

Rectangle {
    id: mainview
    width: 800
    height: 600

    GraphsView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 10
        orientation: Qt.Horizontal

        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Light
            grid.mainColor: "#cccccc"
            grid.subColor: "#dddddd"
        }

        axisX: ValueAxis {
            max: 5
        }
        axisY: ValueAxis {
            max: 10
        }

        Component {
            id: boxplotDelegate
            Item {
                property var data
                Rectangle {
                    color: "black"
                    y: boxplotSeries.mapY(parent.data.index + 1) - height / 2 + 1
                    x: boxplotSeries.mapX(parent.data.median) - width / 2
                    height: 4
                    width: Math.abs(boxplotSeries.mapX(parent.data.upperExtreme) - boxplotSeries.mapX(parent.data.lowerExtreme))
                }
                Rectangle {
                    color: "tan"
                    border.color: "black"
                    border.width: 4
                    y: boxplotSeries.mapY(parent.data.index + 1) - height / 2 + 1
                    x: boxplotSeries.mapX(parent.data.median) - width / 2
                    height: 32
                    width: Math.abs(boxplotSeries.mapX(parent.data.upperQuartile) - boxplotSeries.mapX(parent.data.lowerQuartile))
                }
                Rectangle {
                    color: "black"
                    y: boxplotSeries.mapY(parent.data.index + 1) - height / 2 + 1
                    x: boxplotSeries.mapX(parent.data.upperExtreme) - width / 2
                    height: 32
                    width: 4
                }
                Rectangle {
                    color: "black"
                    y: boxplotSeries.mapY(parent.data.index + 1) - height / 2 + 1
                    x: boxplotSeries.mapX(parent.data.median) - width / 2
                    height: 32
                    width: 4
                }
                Rectangle {
                    color: "black"
                    y: boxplotSeries.mapY(parent.data.index + 1) - height / 2 + 1
                    x: boxplotSeries.mapX(parent.data.lowerExtreme) - width / 2
                    height: 32
                    width: 4
                }
            }
        }

        CustomSeries {
            id: boxplotSeries

            delegate: boxplotDelegate

            CustomSeriesData {
                data: ({
                    upperExtreme: 5,
                    upperQuartile: 4,
                    median: 3,
                    lowerQuartile: 2,
                    lowerExtreme: 1
                })
            }

            CustomSeriesData {
                data: ({
                    upperExtreme: 6,
                    upperQuartile: 5.5,
                    median: 5,
                    lowerQuartile: 4.5,
                    lowerExtreme: 4
                })
            }

            CustomSeriesData {
                data: ({
                    upperExtreme: 5,
                    upperQuartile: 4.5,
                    median: 4,
                    lowerQuartile: 3.5,
                    lowerExtreme: 3
                })
            }

            CustomSeriesData {
                data: ({
                    upperExtreme: 8,
                    upperQuartile: 7,
                    median: 6,
                    lowerQuartile: 5,
                    lowerExtreme: 4
                })
            }
        }
    }

    GraphsView {
        anchors.left: parent.horizontalCenter
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 10

        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Light
            grid.mainColor: "#cccccc"
            grid.subColor: "#dddddd"
        }

        axisX: ValueAxis {
            max: 10
        }
        axisY: ValueAxis {
            max: 100
        }

        Component {
            id: candleDelegate
            Item {
                Rectangle {
                    color: "black"
                    width: 3
                }

                Rectangle {
                    color: "lightgreen"
                    border.color: "black"
                    border.width: 3
                    width: 24
                }
            }
        }

        Candlestick {
            id: candleSeries
            delegate: candleDelegate
        }
    }
}
