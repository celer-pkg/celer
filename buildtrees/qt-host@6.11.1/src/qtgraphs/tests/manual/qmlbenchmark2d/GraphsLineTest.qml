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
            dataSource.reset(1000);
        }

        axisX: ValueAxis {
            min: 0
            max: {
                if (series.count > 0)
                    return series.count
                else
                    return 1000
            }
        }

        axisY: ValueAxis {
            min: 0
            max: 10
        }

        LineSeries {
            id: series
        }
    }

    FrameAnimation {
        running: true
        onTriggered: {
            dataSource.update(series);
        }
    }

    Timer {
        interval: 400; running: true; repeat: true
        onTriggered: {
            parent.measure(series.count)
        }
    }
}
