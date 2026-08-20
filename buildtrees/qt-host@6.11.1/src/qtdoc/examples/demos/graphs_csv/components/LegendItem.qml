// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtGraphs
import qtgraphscsv

Item {
    id: legendItem

    property BarSeries series
    property color labelTextColor
    property color titleTextColor

    readonly property font titleFont: ({
            "family": "Inter",
            "weight": 600 * Units.px,
            "pixelSize": 12 * Units.px,
            "letterSpacing": 0 * Units.px,
            "bold": false
        })

    readonly property font labelFont: ({
            "family": "Inter",
            "weight": 600 * Units.px,
            "pixelSize": 10 * Units.px,
            "letterSpacing": 0 * Units.px,
            "bold": false
        })

    Column {
        id: topLayout

        spacing: 15 * Units.px

        Text {
            id: title
            color: legendItem.titleTextColor
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Selected")
        }

        Flickable {
            id: selectionFlickable

            width: contentWidth
            height: 200 * Units.px

            contentWidth: selectionLabels.width
            contentHeight: selectionLabels.height
            clip: true

            Flow {
                id: selectionLabels

                spacing: 12 * Units.px
                width: 240 * Units.px

                Repeater {
                    id: labelRepeater

                    model: legendItem.series ? legendItem.series.legendData.length : 0

                    Rectangle {
                        id: legend1

                        required property int index

                        height: 20 * Units.px
                        width: text1.width * Units.px
                        radius: 4 * Units.px

                        color: legendItem.series.legendData[index].color

                        Text {
                            id: text1
                            topPadding: 4 * Units.px
                            bottomPadding: 4 * Units.px
                            leftPadding: 6 * Units.px
                            rightPadding: 6 * Units.px
                            horizontalAlignment: Text.AlignHCenter
                            color: legendItem.labelTextColor
                            font: legendItem.labelFont
                            text: legendItem.series.legendData[legend1.index].label
                        }
                    }
                }
            }
        }
    }
}
