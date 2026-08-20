// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    property var bars3d: null

    Bars3D {
        id: bars1
        anchors.fill: parent
    }

    Bars3D {
        id: bars2
        anchors.fill: parent
        customItemList: [item1, item2]
    }

    TestCase {
        name: "Bars3D Series"
        when: windowShown

        Bar3DSeries { id: series1
            dataProxy: ItemModelBarDataProxy {
                itemModel: ListModel {
                    ListElement{ year: "2012"; city: "Oulu"; expenses: "4200"; }
                    ListElement{ year: "2012"; city: "Rauma"; expenses: "2100"; }
                }
                rowRole: "city"
                columnRole: "year"
                valueRole: "expenses"
            }
        }
        Bar3DSeries { id: series2 }

        function test_1_add_series() {
            bars1.seriesList = [series1, series2]
            compare(bars1.seriesList.length, 2)
            console.log("top:", top)
        }

        function test_2_remove_series() {
            bars1.seriesList = [series1]
            compare(bars1.seriesList.length, 1)
        }

        function test_3_remove_series() {
            bars1.seriesList = []
            compare(bars1.seriesList.length, 0)
        }

        function test_4_primary_series() {
            bars1.seriesList = [series1, series2]
            compare(bars1.primarySeries, series1)
            bars1.primarySeries = series2
            compare(bars1.primarySeries, series2)
        }

        function test_5_selected_series() {
            bars1.seriesList[0].selectedBar = Qt.point(0, 0)
            compare(bars1.selectedSeries, series1)
        }

        function test_6_has_series() {
            bars1.seriesList = [series1]
            compare(bars1.hasSeries(series1), true)
            compare(bars1.hasSeries(series2), false)
        }
    }

    Custom3DItem { id: item1; meshFile: ":/customitem.mesh" }
    Custom3DItem { id: item2; meshFile: ":/customitem.mesh" }
    Custom3DItem { id: item3; meshFile: ":/customitem.mesh" }
    Custom3DItem { id: item4; meshFile: ":/customitem.mesh"; position: Qt.vector3d(0.0, 1.0, 0.0) }

    TestCase {
        name: "Bars3D Theme"
        when: windowShown

        GraphsTheme { id: newTheme }

        function test_1_add_theme() {
            bars1.theme = newTheme
            compare(bars1.theme, newTheme)
        }

        function test_2_change_theme() {
            newTheme.theme = GraphsTheme.Theme.QtGreenNeon
            compare(bars1.theme.theme, GraphsTheme.Theme.QtGreenNeon)
        }
    }

    TestCase {
        name: "Bars3D Custom"
        when: windowShown

        function test_1_custom_items() {
            compare(bars2.customItemList.length, 2)
        }

        function test_2_add_custom_items() {
            bars2.addCustomItem(item3)
            compare(bars2.customItemList.length, 3)
            bars2.addCustomItem(item4)
            compare(bars2.customItemList.length, 4)
        }

        function test_3_change_custom_items() {
            item1.position = Qt.vector3d(1.0, 1.0, 1.0)
            compare(bars2.customItemList[0].position, Qt.vector3d(1.0, 1.0, 1.0))
        }

        function test_4_remove_custom_items() {
            bars2.removeCustomItemAt(Qt.vector3d(0.0, 1.0, 0.0))
            compare(bars2.customItemList.length, 3)
            bars2.releaseCustomItem(item1)
            compare(bars2.customItemList[0], item2)
            bars2.releaseCustomItem(item2)
            compare(bars2.customItemList.length, 1)
            bars2.removeCustomItems()
            compare(bars2.customItemList.length, 0)
        }
    }
}
