// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtGraphs
import QtQuick
import AerospaceHubExample

GraphsView {
    id: graph

    property alias series: pieseries2d
    property var labelposition: PieSlice.LabelPosition.InsideHorizontal
    property bool explode
    property bool customcolors: true
    property PieSimulator piesimulator

    theme: GlobalSettings.theme

    visible: opacity > 0.5

    axisX: ValueAxis {
        visible: false
        lineVisible: false
        gridVisible: false
        subGridVisible: false
    }

    axisY: ValueAxis {
        visible: false
        lineVisible: false
        gridVisible: false
        subGridVisible: false
    }

    PieSeries {
        id: pieseries2d
        angleSpanVisibleLimit: 60
        angleSpanLabelVisibility: PieSeries.LabelVisibility.None

        onAdded: (slices) => {
                                      graph.updateSliceVisuals()
                 }
    }

    function updateSliceVisuals() {
                         let slice = pieseries2d.at(0)
                         slice.labelColor = "#F2F2F2"
                         slice.exploded = graph.explode
                         slice.labelVisible = true
                         slice.labelPosition = graph.labelposition
                         slice.borderColor = graph.customcolors ? "#1B308D" : "#00000000"
                         slice.color = graph.customcolors ? "#1B308D" : "#00000000"
                         slice.label = "Critical: " + (slice.angleSpan / 3.6).toFixed(0) + "%"

                         if (!pieseries2d.at(1))
                             return

                         slice = pieseries2d.at(1)
                         slice.labelColor = "#F2F2F2"
                         slice.exploded = graph.explode
                         slice.labelVisible = true
                         slice.labelPosition = graph.labelposition
                         slice.borderColor = graph.customcolors ? "#173DDE" : "#00000000"
                         slice.color = graph.customcolors ? "#173DDE" : "#00000000"
                         slice.label = "Warning: " + (slice.angleSpan / 3.6).toFixed(0) + "%"

                         if (!pieseries2d.at(2))
                             return

                         slice = pieseries2d.at(2)
                         slice.labelColor = "#F2F2F2"
                         slice.exploded = graph.explode
                         slice.labelVisible = true
                         slice.labelPosition = graph.labelposition
                         slice.borderColor = graph.customcolors ? "#3673FC" : "#00000000"
                         slice.color = graph.customcolors ? "#3673FC" : "#00000000"
                         slice.label = "Info: " + (slice.angleSpan / 3.6).toFixed(0) + "%"
    }

    Component.onCompleted: {
        piesimulator.addSeries(pieseries2d)
    }
}
