// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import QtGraphs
import AerospaceHubExample

GraphsView {
    id: graph

    property alias series: barseries2d
    property bool customcolors: true
    property BarSimulator barsimulator

    theme: GlobalSettings.theme

    visible: opacity > 0.5

    onHoverEnter: {
        tooltip.visible = true
    }

    onHoverExit: {
        tooltip.visible = false
    }

    onHover: (seriesName, position, value) => {
        tooltip.x = position.x
        tooltip.y = position.y - tooltip.height
        tooltip.first = barseries2d.barSets[0].values[value.x].toFixed(1)
        tooltip.second = barseries2d.barSets[1].values[value.x].toFixed(1)
        tooltip.third = barseries2d.barSets[2].values[value.x].toFixed(1)
    }

    ToolTip2D {
        id: tooltip
    }

    axisX: BarCategoryAxis {
        gridVisible: false
        subGridVisible: false
        categories: ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]
        titleText: "Month"
    }

    axisY: ValueAxis {
        max: 100
        tickInterval: 20
        titleText: "Energy production"
    }

    BarSeries {
        id: barseries2d
        hoverable: true
        barsType: BarSeries.BarsType.Stacked
        barDelegate: Rectangle {
            property color barColor
            property real barValue

            gradient: Gradient {
                GradientStop { position: 0.0; color: barColor }
                GradientStop { position: 1.0; color: barColor.darker(1.2) }
            }

            Label {
                anchors.centerIn: parent
                text: parent.barValue.toFixed(1)
                color: GlobalSettings.theme.labelTextColor
                visible: barseries2d.labelsVisible
                rotation: barseries2d.labelsAngle
            }
        }

        seriesColors: graph.customcolors ? [
                                         "#1B308D",
                                         // "#1932B4",
                                         "#173DDE",
                                         // "#1F52F1",
                                         "#3673FC"
                                     ] : []
    }

    Component.onCompleted: {
        barsimulator.addSeries(barseries2d)
    }
}
