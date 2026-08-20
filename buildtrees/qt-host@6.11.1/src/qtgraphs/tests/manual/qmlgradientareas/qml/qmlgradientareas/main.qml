// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes
import QtGraphs
import "."

Item {
    id: mainview

    width: 1280
    height: 1024

    GridLayout {
        anchors.fill: parent
        columns: 2

        GraphsView {
            id: graph

            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: ValueAxis {
                    min: 0
                    max: 10
                    titleText: "X Axis"
                }

            axisY: ValueAxis {
                    min: 0
                    max: 10
                    titleText: "Y Axis"
                }

            AreaSeries {
                color: "#cc88aaff"
                gradient: LinearGradient {
                    x1: 0; y1: 0
                    x2: 0; y2: graph.plotArea.height
                    stops: [
                        GradientStop { position: 0.0; color: "red" },
                        GradientStop { position: 1.0; color: "white" }
                    ]
                }

                upperSeries: LineSeries {
                    XYPoint { x: 0; y: 2 }
                    XYPoint { x: 2; y: 4 }
                    XYPoint { x: 4; y: 3 }
                    XYPoint { x: 6; y: 7 }
                    XYPoint { x: 8; y: 6 }
                    XYPoint { x: 10; y: 8 }
                }
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: ValueAxis {
                    min: 0
                    max: 10
                    titleText: "X Axis"
                }

            axisY: ValueAxis {
                    min: 0
                    max: 10
                    titleText: "Y Axis"
                }

            AreaSeries {
                color: "#cc88aaff"
                selected: true
                selectedGradient: ConicalGradient {
                    centerX: 0.5
                    centerY: 0.5
                    angle: 0
                    stops: [
                        GradientStop { position: 0.0; color: "transparent" },
                        GradientStop { position: 0.95; color: "blue" }
                    ]
                }

                upperSeries: LineSeries {
                    XYPoint { x: 0; y: 2 }
                    XYPoint { x: 2; y: 4 }
                    XYPoint { x: 4; y: 3 }
                    XYPoint { x: 6; y: 7 }
                    XYPoint { x: 8; y: 6 }
                    XYPoint { x: 10; y: 8 }
                }

                lowerSeries: LineSeries {
                    XYPoint { x: 0; y: 1 }
                    XYPoint { x: 2; y: 2 }
                    XYPoint { x: 4; y: 2 }
                    XYPoint { x: 6; y: 4 }
                    XYPoint { x: 8; y: 3 }
                    XYPoint { x: 10; y: 5 }
                }
            }
        }
    }
}
