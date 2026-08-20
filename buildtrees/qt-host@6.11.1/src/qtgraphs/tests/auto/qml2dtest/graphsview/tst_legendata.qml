// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150
    GraphsView {
        id: graphsView
        height: top.height
        width: top.width

        axisX: ValueAxis {
            id: xAxis
            max: 4
        }
        axisY: ValueAxis {
            id: yAxis
            max: 8
        }

        theme: GraphsTheme {
            id: myTheme
            theme: GraphsTheme.Theme.QtGreen
            axisXLabelFont.pixelSize: 20
            axisYLabelFont.pixelSize: 16
        }

        BarSeries {
            id: barInitial
        }

        BarSeries {
            id: barInitialized
            seriesColors: ["#ff0000"]
            borderColors: ["#00ff00"]
            name: "BarSeries"
            visible: false

            BarSet {
                label: "Set1"
                values: [1, 2, 3, 4, 5, 6]
            }
        }
        BarSeries {
            id: barInitialized2
            seriesColors: ["#0000ff", "#ff00ff"]
            borderColors: ["#00ffff", "#ffff00"]
            name: "BarSeries"
            visible: false

            BarSet {
                label: "Set1"
                values: [1, 2, 3, 4, 5, 6]
            }
            BarSet {
                label: "Set2"
                values: [1, 2, 3, 4, 5, 6]
            }
        }
        LineSeries {
            id: lineInitial
            XYPoint {
                x: 0
                y: 6.6
            }
        }

        LineSeries {
            id: lineInitialized
            name: "Second"

            XYPoint {
                x: 0
                y: 6.6
            }
            XYPoint {
                x: 0.6
                y: 4.1
            }
            XYPoint {
                x: 1.5
                y: 5.3
            }
            XYPoint {
                x: 2.2
                y: 7.1
            }
            XYPoint {
                x: 3.3
                y: 6.9
            }
            XYPoint {
                x: 3.6
                y: 5.0
            }
            XYPoint {
                x: 4.0
                y: 5.3
            }
        }

        PieSeries {
            id: pieInitial
        }

        PieSeries {
            id: pieInitialized

            PieSlice {
                label: "Volkswagen"
                labelVisible: true
                value: 13.5
            }
            PieSlice {
                label: "Toyota"
                labelVisible: true
                labelPosition: PieSlice.LabelPosition.InsideHorizontal
                value: 10.9
            }
        }
        AreaSeries {
            id: areaInitial
        }
        AreaSeries {
            id: areaInitialized
            color: "#ff0000"
            borderColor: "#00ff00"
            upperSeries: SplineSeries {
                XYPoint {
                    x: 6
                    y: 1
                }
                XYPoint {
                    x: 7
                    y: 0.5
                }
                XYPoint {
                    x: 8
                    y: 2
                }
            }
            name: "First"
        }
        AreaSeries {
            id: areaInitialized2
            color: "#aabbcc"
            borderColor: "#bbffcc"

            upperSeries: SplineSeries {
                XYPoint {
                    x: 6
                    y: 2
                }
                XYPoint {
                    x: 7
                    y: 3.5
                }
                XYPoint {
                    x: 8
                    y: 3.8
                }
            }

            lowerSeries: SplineSeries {
                XYPoint {
                    x: 6.4
                    y: 1.5
                }
                XYPoint {
                    x: 7
                    y: 2.5
                }
                XYPoint {
                    x: 8
                    y: 3
                }
            }
            name: "Second"
        }
    }

    GraphsTheme {
        id: theme1
        seriesColors: ["#ff0000"]
        borderColors: ["#00ff00"]
    }

    GraphsTheme {
        id: theme2
        seriesColors: ["#0000ff", "#ff00ff"]
        borderColors: ["#00ffff", "#ffff00"]
    }

    TestCase {
        name: "LegendData BarsRenderer Initial"

        function test_1_initial() {
            // Properties from QBarSeries
            compare(barInitial.legendData.length, 0)
        }

        function test_1_initial_change() {
            waitForRendering(top)
            compare(barInitial.legendData.length, 0)
        }
    }

    TestCase {
        name: "LegendData BarsRenderer Initialized"

        function test_1_initialized() {
            waitForRendering(top)
            compare(barInitialized.legendData.length, 1)

            compare(barInitialized.legendData[0].color, "#ff0000")
            compare(barInitialized.legendData[0].borderColor, "#00ff00")
            compare(barInitialized2.legendData[0].label, "Set1")
        }

        function test_2_initialized() {
            compare(barInitialized2.legendData.length, 2)

            compare(barInitialized2.legendData[0].color, "#0000ff")
            compare(barInitialized2.legendData[0].borderColor, "#00ffff")
            compare(barInitialized2.legendData[0].label, "Set1")

            compare(barInitialized2.legendData[1].color, "#ff00ff")
            compare(barInitialized2.legendData[1].borderColor, "#ffff00")
            compare(barInitialized2.legendData[1].label, "Set2")
        }

        function test_3_initialized_change() {
            // Reset to empty so that theme takes over
            barInitialized.seriesColors = []
            barInitialized.borderColors = []
            graphsView.theme = theme2
            waitForRendering(top)
            compare(barInitialized.legendData.length, 1)

            compare(barInitialized.legendData[0].color, "#0000ff")
            compare(barInitialized.legendData[0].borderColor, "#00ffff")
            compare(barInitialized2.legendData[0].label, "Set1")

            // Reset to empty so that theme takes over
            barInitialized2.seriesColors = []
            barInitialized2.borderColors = []
            graphsView.theme = theme1
            waitForRendering(top)
            compare(barInitialized2.legendData.length, 2)

            compare(barInitialized2.legendData[0].color, "#ff0000")
            compare(barInitialized2.legendData[0].borderColor, "#00ff00")
            compare(barInitialized2.legendData[0].label, "Set1")

            compare(barInitialized2.legendData[1].color, "#ff0000")
            compare(barInitialized2.legendData[1].borderColor, "#00ff00")
            compare(barInitialized2.legendData[1].label, "Set2")
        }
    }
    TestCase {
        name: "LegendData PointRenderer Initial"

        function test_1_Initial() {
            graphsView.theme = myTheme
            waitForRendering(top)
            compare(lineInitial.legendData.length, 1)

            compare(lineInitial.legendData[0].color, "#7be6b1")
            compare(lineInitial.legendData[0].borderColor, "#7be6b1")
            compare(lineInitial.legendData[0].label, "")
        }
        function test_2_Initial_change() {
            graphsView.theme = theme1
            waitForRendering(top)

            compare(lineInitial.legendData.length, 1)

            compare(lineInitial.legendData[0].color, "#ff0000")
            compare(lineInitial.legendData[0].borderColor, "#00ff00")
            compare(lineInitial.legendData[0].label, "")
        }
    }

    TestCase {
        name: "LegendData PointRenderer Initialized"

        function test_1_Initialized() {
            waitForRendering(top)
            compare(lineInitialized.legendData.length, 1)

            compare(lineInitialized.legendData[0].color, "#ff0000")
            compare(lineInitialized.legendData[0].borderColor, "#00ff00")
            compare(lineInitialized.legendData[0].label, "Second")
        }

        function test_2_Initialized_Change() {
            graphsView.theme = theme2
            waitForRendering(top)
            compare(lineInitialized.legendData.length, 1)

            compare(lineInitialized.legendData[0].color, "#ff00ff")
            compare(lineInitialized.legendData[0].borderColor, "#ffff00")
            compare(lineInitialized.legendData[0].label, "Second")
        }
    }

    TestCase {
        name: "LegendData PieRenderer Initial"

        function test_1_initial() {
            waitForRendering(top)

            compare(pieInitial.legendData.length, 0)
        }

        function test_2_initial_change() {
            graphsView.theme = theme1
            waitForRendering(top)
            compare(pieInitial.legendData.length, 0)
        }
    }

    TestCase {
        name: "LegendData PieRenderer Initialized"

        function test_1_initialized() {
            waitForRendering(top)
            compare(pieInitialized.legendData.length, 2)

            compare(pieInitialized.legendData[0].color, "#ff0000")
            compare(pieInitialized.legendData[0].borderColor, "#00ff00")
            compare(pieInitialized.legendData[0].label, "Volkswagen")

            compare(pieInitialized.legendData[1].color, "#ff0000")
            compare(pieInitialized.legendData[1].borderColor, "#00ff00")
            compare(pieInitialized.legendData[1].label, "Toyota")
        }

        function test_2_initialized_change() {
            graphsView.theme = theme2
            waitForRendering(top)
            waitForPolish(top)
            compare(pieInitialized.legendData.length, 2)

            compare(pieInitialized.legendData[0].color, "#0000ff")
            compare(pieInitialized.legendData[0].borderColor, "#00ffff")
            compare(pieInitialized.legendData[0].label, "Volkswagen")

            compare(pieInitialized.legendData[1].color, "#ff00ff")
            compare(pieInitialized.legendData[1].borderColor, "#ffff00")
            compare(pieInitialized.legendData[1].label, "Toyota")
        }
    }
    TestCase {
        name: "LegendData AreaSeries Initial"
        function initTestCase() {
            waitForRendering(top)
        }

        function test_1_initial() {
            compare(areaInitial.legendData.length, 0)
        }

        function test_2_initial_change() {
            graphsView.theme = theme1
            waitForRendering(top)

            compare(areaInitial.legendData.length, 0)
        }
    }
    TestCase {
        name: "LegendData AreaSeries Initialized"
        function initTestCase() {
            waitForRendering(top)
        }

        function test_1_initialized() {
            compare(areaInitialized.legendData.length, 1)
            compare(areaInitialized2.legendData.length, 1)

            compare(areaInitialized.legendData[0].color, "#ff0000")
            compare(areaInitialized.legendData[0].borderColor, "#00ff00")
            compare(areaInitialized.legendData[0].label, "First")

            compare(areaInitialized2.legendData[0].color, "#aabbcc")
            compare(areaInitialized2.legendData[0].borderColor, "#bbffcc")
            compare(areaInitialized2.legendData[0].label, "Second")
        }

        function test_2_initialized_change() {
            // Reset to transparent so that theme takes over
            areaInitialized.color = "#00000000"
            areaInitialized.borderColor = "#00000000"
            graphsView.theme = theme2
            areaInitialized2.color = "#ffffff"
            areaInitialized2.borderColor = "#dddddd"
            waitForRendering(top)

            compare(areaInitialized.legendData.length, 1)
            compare(areaInitialized2.legendData.length, 1)

            compare(areaInitialized.legendData[0].color, "#0000ff")
            compare(areaInitialized.legendData[0].borderColor, "#00ffff")
            compare(areaInitialized.legendData[0].label, "First")

            compare(areaInitialized2.legendData[0].color, "#ffffff")
            compare(areaInitialized2.legendData[0].borderColor, "#dddddd")
            compare(areaInitialized2.legendData[0].label, "Second")
        }
    }
}
