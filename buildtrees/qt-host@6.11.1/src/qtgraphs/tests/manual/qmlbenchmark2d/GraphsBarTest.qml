// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs

Rectangle {
    signal measure(int count)
    width: 800
    height: 600
    color: "#404040"

    GraphsView {
        id: graph
        anchors.fill: parent
        antialiasing: true

        Component.onCompleted: {
            dataSource.reset(10);
        }

        axisX: BarCategoryAxis {
            id: xAxis
        }

        axisY: ValueAxis {
            min: 0
            max: 10
        }

        BarSeries {
            id: series
        }
    }

    FrameAnimation {
        id: fA
        running: true
        onTriggered: {
            dataSource.update(series);
            if (series.barSets[0]) {
                while (xAxis.categories.length < series.barSets[0].count)
                    xAxis.categories.push(xAxis.categories.length.toString());
            }
        }
    }

    Timer {
        interval: 400; running: true; repeat: true
        onTriggered: {
            parent.measure(series.barSets[0].count)
        }
    }
}
