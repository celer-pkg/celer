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
            chartsDataSource.reset(1000);
        }

        LineSeries {
            id: series

            axisX: ValueAxis {
                id: xAxis
                min: 0
                max: series.count
            }

            axisY: ValueAxis {
                min: 0
                max: 10
            }
        }
    }

    FrameAnimation {
        running: true
        onTriggered: {
            if (xAxis.max !== series.count)
                xAxis.max = series.count
            chartsDataSource.update(series);
        }
    }

    Timer {
        interval: 400; running: true; repeat: true
        onTriggered: {
            parent.measure(series.count)
        }
    }
}
