// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import QtGraphs
import qtgraphscsv

Item {
    id: graphsItem

    property alias series: graphsView.barSeries
    property alias chartView: graphsView
    property alias modelMapper: graphsView.modelMapper
    property alias categoryAxis: graphsView.categoryAxis
    property alias theme: graphsView.theme
    property alias labelDelegateTextColor: graphsView.labelTextColor

    readonly property font graphCategoryFont: ({
            "family": "Inter",
            "weight": 600 * Units.px,
            "pixelSize": 14 * Units.px,
            "letterSpacing": 0 * Units.px,
            "bold": false
        })

    function updateModelMapper(first, last) {
        chartView.modelMapper.firstBarSetSection = first.y;
        chartView.modelMapper.lastBarSetSection = last.y;
        chartView.modelMapper.first = first.x;
        chartView.modelMapper.count = (last.x - first.x) + 1;
    }

    function clearGraph() {
        series.clear();
        categoryAxis.clear();
    }

    GraphsView {
        id: graphsView
        anchors.fill: graphsItem

        property alias modelMapper: barModelMapper
        property alias barSeries: barSeries
        property alias categoryAxis: barCategoryAxis
        property alias barRadius: barSeries.radius
        property alias barBlur: barSeries.blur
        property alias opa: barSeries.opa
        property alias sizeFactor: barSeries.sizeFactor
        property color labelTextColor: "green"

        marginLeft: 71 * Units.px
        marginRight: 43 * Units.px
        marginTop: 47 * Units.px
        marginBottom: 80 * Units.px

        axisX: BarCategoryAxis {
            id: barCategoryAxis
            subGridVisible: false

            property color labelDelegateTextColor: graphsView.labelTextColor

            labelDelegate: Item {
                id: labelItem
                property string text
                property color labelTextColor: barCategoryAxis.labelDelegateTextColor

                Text {
                    id: labelDelegate
                    anchors.centerIn: parent
                    font: graphsItem.graphCategoryFont
                    text: labelItem.text
                    color: labelItem.labelTextColor
                }
            }
        }

        axisY: ValueAxis {
            id: axisY
            max: 35
            min: 0
            subTickCount: 4
            tickInterval: 5
        }

        BarSeries {
            id: barSeries
            property real radius: 20 * Units.px
            property real blur: 15
            property real sizeFactor: 0.6
            property real opa: 0.4

            barDelegate: Item {
                id: customBar

                property color barColor

                RectangularShadow {
                    id: shadowEffect

                    anchors.horizontalCenter: barRectangle.horizontalCenter
                    anchors.bottom: barRectangle.bottom
                    blur: barSeries.blur
                    opacity: barSeries.opa
                    width: barSeries.sizeFactor * barRectangle.width * Units.px
                    height: 0.9 * Math.min(barRectangle.height, 600) * Units.px
                    cached: true
                }

                Rectangle {
                    id: barRectangle
                    anchors.fill: parent
                    color: customBar.barColor
                    opacity: 0.9
                    width: 24 * Units.px
                    topLeftRadius: barSeries.radius
                    topRightRadius: barSeries.radius
                }
            }
        }

        BarModelMapper {
            id: barModelMapper
            series: barSeries
            firstBarSetSection: -1
            lastBarSetSection: -1
            first: -1
            count: -1
            orientation: Qt.Horizontal
        }
    }
}
