// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Surface3D {
        id: series
        anchors.fill: parent
    }

    TestCase {
        name: "Surface3D Series"

        Surface3DSeries { id: series1 }
        Surface3DSeries { id: series2 }

        Custom3DItem { id: customItem1 }
        Custom3DItem { id: customItem2 }
        Custom3DItem { id: customItem3 }

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
            series.seriesList[0].selectedPoint = Qt.point(0, 0)
            compare(series.selectedSeries, series1)
        }

        function test_5_has_series() {
            series.seriesList = [series1]
            compare(series.hasSeries(series1), true)
            compare(series.hasSeries(series2), false)
        }

        function test_6_add_custom_item() {
            series.addCustomItem(customItem1)
            series.addCustomItem(customItem2)
            series.addCustomItem(customItem3)
            compare(series.customItemList.length, 3)
        }

        function test_7_remove_custom_item() {
            series.releaseCustomItem(customItem1)
            compare(series.customItemList.length, 2)

            verify(customItem1)
            // Add item back
            series.addCustomItem(customItem1)
            compare(series.customItemList.length, 3)

            // Remove items, this also deletes them.
            series.removeCustomItem(customItem1)
            compare(series.customItemList.length, 2)

            series.removeCustomItems()
            compare(series.customItemList.length, 0)

            verify(!customItem1)
            verify(!customItem2)
            verify(!customItem3)
        }
    }
}
