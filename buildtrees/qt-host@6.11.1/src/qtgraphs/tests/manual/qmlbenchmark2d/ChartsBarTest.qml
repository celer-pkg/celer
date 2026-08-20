// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtCharts

Rectangle {
    signal measure(int count)
    width: 800
    height: 600
    color: "#404040"

    ChartView {
        anchors.fill: parent
        antialiasing: true

        Component.onCompleted: {
            chartsDataSource.reset(10);
        }

        BarSeries {
            id: series

            axisX: BarCategoryAxis {
                id: xAxis
            }

            axisY: ValueAxis {
                min: 0
                max: 10
            }

            BarSet {
            }
        }
    }

    FrameAnimation {
        id: fA
        running: true
        onTriggered: {
            chartsDataSource.update(series);
            if (series.at(0)) {
                while (xAxis.categories.length < series.at(0).count)
                    xAxis.categories.push(xAxis.categories.length.toString());
            }
        }
    }

    Timer {
        interval: 400; running: true; repeat: true
        onTriggered: {
            if (series.at(0))
                parent.measure(series.at(0).count)
        }
    }
}
