// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtGraphs
import "."

Item {
    id: mainview

    width: 1280
    height: 1024

    BarCategoryAxis {
        id: bar
        categories: ["2020", "2021", "2022", "2023", "2024", "2025"]
    }

    GridLayout {
        anchors.fill: parent
        columns: 2

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            axisX: bar
            axisY: ValueAxis {
                max: 30
            }

            BarSeries {
                BarSet {
                    label: "Alice"
                    values: [8, 2, 3, 4, 5, 6]
                }

                BarSet {
                    label: "Bob"
                    values: [5, 1, 2, 4, 1, 7]
                }

                BarSet {
                    label: "Carol"
                    values: [3, 5, 8, 13, 5, 8]
                }
            }

            BarSeries {
                seriesColors: ["red", "blue", "green"]

                BarSet {
                    label: "Carol"
                    values: [2, 2, 3, 4, 5, 6]
                }

                BarSet {
                    label: "Bob"
                    values: [5, 1, 2, 4, 1, 7]
                }

                BarSet {
                    label: "Trent"
                    values: [3, 5, 8, 13, 5, 8]
                }
            }

            BarSeries {
                barsType: BarSeries.BarsType.Stacked
                seriesColors: ["cyan", "yellow", "salmon"]

                BarSet {
                    label: "Carol"
                    values: [2, 2, 3, 4, 5, 6]
                }

                BarSet {
                    label: "Bob"
                    values: [5, 1, 2, 4, 1, 7]
                }

                BarSet {
                    label: "Trent"
                    values: [3, 5, 8, 13, 5, 8]
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            axisX: bar

            BarSeries {
                barsType: BarSeries.BarsType.Stacked

                axisY: ValueAxis {
                    max: 30
                }

                BarSet {
                    label: "Alice"
                    values: [8, 2, 3, 4, 5, 6]
                }

                BarSet {
                    label: "Bob"
                    values: [5, 1, 2, 4, 1, 7]
                }

                BarSet {
                    label: "Carol"
                    values: [3, 5, 8, 13, 5, 8]
                }
            }

            BarSeries {
                barsType: BarSeries.BarsType.StackedPercent
                seriesColors: ["red", "blue", "green"]

                axisY: ValueAxis {
                    max: 100
                }

                BarSet {
                    label: "Carol"
                    values: [2, 2, 3, 4, 5, 6]
                }

                BarSet {
                    label: "Bob"
                    values: [5, 1, 2, 4, 1, 7]
                }

                BarSet {
                    label: "Trent"
                    values: [3, 5, 8, 13, 5, 8]
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            axisX: bar

            orientation: Qt.Horizontal
            BarSeries {
                axisY: ValueAxis {
                    max: 20
                }

                BarSet {
                    label: "Alice"
                    values: [8, 2, 3, 4, 5, 6]
                }

                BarSet {
                    label: "Bob"
                    values: [5, 1, 2, 4, 1, 7]
                }

                BarSet {
                    label: "Carol"
                    values: [3, 5, 8, 13, 5, 8]
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            axisX: bar

            orientation: Qt.Horizontal
            BarSeries {
                axisY: ValueAxis {
                    max: 20
                }

                BarSet {
                    label: "Alice"
                    values: [8, 2, 3, 4, 5, 6]
                }

                BarSet {
                    label: "Bob"
                    values: [5, 1, 2, 4, 1, 7]
                }

                BarSet {
                    label: "Carol"
                    values: [3, 5, 8, 13, 5, 8]
                }
            }

            BarSeries {
                barsType: BarSeries.BarsType.StackedPercent

                axisY: ValueAxis {
                    max: 100
                }

                BarSet {
                    label: "Alice"
                    values: [8, 2, 3, 4, 5, 6]
                }

                BarSet {
                    label: "Bob"
                    values: [5, 1, 2, 4, 1, 7]
                }

                BarSet {
                    label: "Carol"
                    values: [3, 5, 8, 13, 5, 8]
                }
            }
        }
    }
}
