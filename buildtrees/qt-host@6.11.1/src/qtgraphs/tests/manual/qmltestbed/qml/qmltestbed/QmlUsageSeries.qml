// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs

Rectangle {
    id: main
    anchors.fill: parent

    GraphsView {
        id: graph
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: col.left

        axisX: BarCategoryAxis {
            categories: ["1", "2", "3", "4", "5", "6", "7", "8"]
        }
        axisY: ValueAxis {
            id: yAxis
            max: 100
        }

        AreaSeries {
            id: area

            upperSeries: SplineSeries {
                id: areaSpline

                XYPoint {x: 0; y:0}
            }

            lowerSeries: LineSeries {
                id: areaLine

                XYPoint {x: 0; y:0}
            }
        }

        LineSeries {
            id: line

            XYPoint {x: 0; y:0}
        }

        ScatterSeries {
            id: scatter

            XYPoint {x: 0; y:0}
        }

        BarSeries {
            id: bar
        }
    }

    Rectangle {
        property int nx: 1

        id: col
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 150

        ColumnLayout {
            Button {
                text: "Append"
                Layout.margins: 20

                onClicked: {
                    if (col.nx > 8)
                        return

                    line.append(col.nx, Math.random() * 100)
                    scatter.append(col.nx, Math.random() * 100)

                    let b = Qt.createQmlObject(`
                                               import QtGraphs;

                                               BarSet {
                                                   values: [
                                                       ${Math.random() * 50},
                                                       ${Math.random() * 50},
                                                       ${Math.random() * 50},
                                                       ${Math.random() * 50},
                                                       ${Math.random() * 50},
                                                       ${Math.random() * 50},
                                                       ${Math.random() * 50},
                                                       ${Math.random() * 50}
                                                    ]
                                                }`,
                                               graph
                                              )

                    bar.append(b)

                    areaSpline.append(col.nx, Math.random() * 15 + 60)
                    areaLine.append(col.nx, Math.random() * 15 + 40)

                    col.nx++
                }
            }

            Button {
                text: "Remove"
                Layout.margins: 20

                onClicked: {
                    if (col.nx <= 1)
                        return

                    line.remove(line.count - 1)
                    scatter.remove(scatter.count - 1)
                    bar.remove(bar.barSets.count - 1)
                    areaSpline.remove(areaSpline.count - 1)
                    areaLine.remove(areaLine.count - 1)

                    col.nx--
                }
            }

            Button {
                text: "Clear"
                Layout.margins: 20

                onClicked: {
                    line.clear()
                    scatter.clear()
                    bar.clear()
                    areaSpline.clear()
                    areaLine.clear()

                    col.nx = 1
                }
            }
        }
    }
}
