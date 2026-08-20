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

    GridLayout {
        anchors.fill: parent
        columns: 3

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: BarCategoryAxis {
                titleText: "Type"
                categories: ["Only positive", "Only negative", "Mixed"]
            }
            axisY: ValueAxis {
                titleText: "Profits"
                min: -60
                max: 50
                tickInterval: 10
            }

            BarSeries {
                BarSet {
                    label: "Dept. A"
                    values: [40, -20, 80]
                }
                BarSet {
                    label: "Dept. B"
                    values: [70, -40, -50]
                }
                BarSet {
                    label: "Dept. C"
                    values: [10, -10, -10]
                }
                BarSet {
                    label: "Dept. D"
                    values: [10, -10, 10]
                }
            }

            BarSeries {
                BarSet {
                    label: "Dept. A"
                    values: [5, 20, 15]
                }
                BarSet {
                    label: "Dept. B"
                    values: [10, 15, 5]
                }
                BarSet {
                    label: "Dept. C"
                    values: [15, 10, 10]
                }
                BarSet {
                    label: "Dept. D"
                    values: [20, 5, 20]
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: BarCategoryAxis {
                titleText: "Type"
                categories: ["Only positive", "Only negative", "Mixed"]
            }
            axisY: ValueAxis {
                titleText: "Profits"
                min: -200
                max: 200
                tickInterval: 20
                subTickCount: 1
            }

            BarSeries {
                barsType: BarSeries.BarsType.Stacked
                BarSet {
                    label: "Dept. A"
                    values: [40, -20, 80]
                }
                BarSet {
                    label: "Dept. B"
                    values: [70, -40, -50]
                }
                BarSet {
                    label: "Dept. C"
                    values: [10, -10, -10]
                }
                BarSet {
                    label: "Dept. D"
                    values: [10, -10, 10]
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: BarCategoryAxis {
                titleText: "Type"
                categories: ["Only positive", "Only negative", "Mixed"]
            }
            axisY: ValueAxis {
                titleText: "Profits"
                min: -200
                max: 200
                tickInterval: 20
                subTickCount: 1
            }

            orientation: Qt.Horizontal

            BarSeries {
                barsType: BarSeries.BarsType.Stacked
                BarSet {
                    label: "Dept. A"
                    values: [40, -20, 80]
                }
                BarSet {
                    label: "Dept. B"
                    values: [70, -40, -50]
                }
                BarSet {
                    label: "Dept. C"
                    values: [10, -10, -10]
                }
                BarSet {
                    label: "Dept. D"
                    values: [10, -10, 10]
                }
            }

            BarSeries {
                BarSet {
                    label: "Dept. A"
                    values: [5, 20, 15]
                }
                BarSet {
                    label: "Dept. B"
                    values: [10, 15, 5]
                }
                BarSet {
                    label: "Dept. C"
                    values: [15, 10, 10]
                }
                BarSet {
                    label: "Dept. D"
                    values: [20, 5, 20]
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: BarCategoryAxis {
                titleText: "Type"
                categories: ["Only positive", "Only negative", "Mixed"]
            }
            axisY: ValueAxis {
                titleText: "Profits"
                min: -60
                max: 50
                tickInterval: 10
            }

            orientation: Qt.Horizontal

            BarSeries {
                BarSet {
                    label: "Dept. A"
                    values: [40, -20, 80]
                }
                BarSet {
                    label: "Dept. B"
                    values: [70, -40, -50]
                }
                BarSet {
                    label: "Dept. C"
                    values: [10, -10, -10]
                }
                BarSet {
                    label: "Dept. D"
                    values: [10, -10, 10]
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: BarCategoryAxis {
                titleText: "Type"
                categories: ["Only positive", "Only negative", "Mixed"]
            }
            axisY: ValueAxis {
                titleText: "Profits"
                min: -110
                max: 110
                tickInterval: 10
            }

            BarSeries {
                barsType: BarSeries.BarsType.StackedPercent
                BarSet {
                    label: "Dept. A"
                    values: [40, -20, 80]
                }
                BarSet {
                    label: "Dept. B"
                    values: [70, -40, -50]
                }
                BarSet {
                    label: "Dept. C"
                    values: [10, -10, -10]
                }
                BarSet {
                    label: "Dept. D"
                    values: [10, -10, 10]
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: BarCategoryAxis {
                titleText: "Type"
                categories: ["Only positive", "Only negative", "Mixed"]
            }
            axisY: ValueAxis {
                titleText: "Profits"
                min: -100
                max: 100
                tickInterval: 10
            }

            orientation: Qt.Horizontal

            BarSeries {
                barsType: BarSeries.BarsType.StackedPercent
                BarSet {
                    label: "Dept. A"
                    values: [40, -20, 80]
                }
                BarSet {
                    label: "Dept. B"
                    values: [70, -40, -50]
                }
                BarSet {
                    label: "Dept. C"
                    values: [10, -10, -10]
                }
                BarSet {
                    label: "Dept. D"
                    values: [10, -10, 10]
                }
            }
        }
    }
}
