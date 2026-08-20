// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import TestbedExample

Rectangle {
    anchors.fill: parent
    color: "#404040"

    GraphsView {
        id: chartView
        anchors.fill: parent
        anchors.margins: 20 * px
        anchors.topMargin: 80 * px
        axisX: BarCategoryAxis {
            categories: ["2023", "2024"]
        }
        axisY: ValueAxis {
            max: 30
        }
        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.QtGreen
            axisXLabelFont.pixelSize: 20
        }
        CustomBarSeries {
            id: barSeries
        }
    }
}
