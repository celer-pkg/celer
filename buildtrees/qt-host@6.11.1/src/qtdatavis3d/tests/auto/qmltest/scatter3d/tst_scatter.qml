// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Scatter3D {
        id: series
        anchors.fill: parent
    }

    TestCase {
        name: "Scatter3D Series"

        Scatter3DSeries { id: series1 }
        Scatter3DSeries { id: series2 }

        function test_1_add_series() {
            series.seriesList = [series1, series2]
            compare(series.seriesList.length, 2)
        }

        function test_2_remove_series() {
            series.seriesList = [series1]
            compare(series.seriesList.length, 1)
        }

        function test_3_remove_series() {
            series.seriesList = []
            compare(series.seriesList.length, 0)
        }

        function test_4_selected_series() {
            series.seriesList = [series1, series2]
            series.seriesList[0].selectedItem = 0
            compare(series.selectedSeries, series1)
        }

        function test_5_has_series() {
            series.seriesList = [series1]
            compare(series.hasSeries(series1), true)
            compare(series.hasSeries(series2), false)
        }
    }
}
