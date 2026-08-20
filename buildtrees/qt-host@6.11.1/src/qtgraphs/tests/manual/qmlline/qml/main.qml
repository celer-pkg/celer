// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtGraphs
import QtQuick
import QtQuick.Layouts
import "."

Item {
    id: mainView
    width: 640
    height: 480
    visible: true

    RowLayout {
        anchors.fill: parent

        GraphsView {
            Layout.fillHeight: true
            Layout.fillWidth: true

            axisX: ValueAxis {
                max: 5
            }
            axisY: ValueAxis {
                max: 5
            }

            LineSeries {
                color: "#00ff00"
                joinStyle: Qt.RoundJoin
                pointDelegate: Rectangle {
                    border.color: Qt.black
                    border.width: 2
                    height: width
                    radius: width / 2
                    width: 10
                }

                XYPoint {
                    x: 0.5
                    y: 0.5
                }

                XYPoint {
                    x: 1.1
                    y: NaN
                }

                XYPoint {
                    x: 1.9
                    y: 3.3
                }

                XYPoint {
                    x: 2.1
                    y: 2.1
                }

                XYPoint {
                    x: 5
                    y: 4.9
                }
            }
        }

        GraphsView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            seriesList: cppLineSeries

            axisX: ValueAxis {
                max: 5
            }
            axisY: ValueAxis {
                max: 5
            }
        }
    }
}
