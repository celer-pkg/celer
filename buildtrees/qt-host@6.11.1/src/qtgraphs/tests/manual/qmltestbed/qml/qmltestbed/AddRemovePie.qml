
// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Controls
import QtGraphs

Item {

    Component {
        id: pieComponent

        PieSeries {
            id: pieSeries
        }
    }

    GraphsView {
        id: graphView
        width: parent.width
        height: parent.height
        anchors.top: title.bottom
        anchors.bottom: parent.bottom

        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.QtGreen
        }
    }

    SettingsView {
        id: sView
        property double radius: 0.8

        Button {
            width: 200
            text: "Add series"
            onClicked: {
                if (sView.radius < 0.1)
                    return

                let pie = pieComponent.createObject(graphView)

                for (var i = 0; i < Math.random() * 10 + 1; ++i) {
                    let slice = pie.append("value", Math.random() * i)
                    slice.labelVisible = true
                }

                pie.pieSize = sView.radius
                sView.radius -= 0.1

                graphView.addSeries(pie)
            }
        }

        Button {
            text: "Toggle last series"

            onClicked: {
                if (graphView.seriesList.length > 0) {
                    let series = graphView.seriesList[graphView.seriesList.length - 1]
                    series.visible = !series.visible
                }
            }
        }

        Button {
            text: "Remove last series"

            onClicked: {
                if (graphView.seriesList.length > 0)
                    sView.radius += 0.1

                graphView.removeSeries(graphView.seriesList.length - 1)
            }
        }
    }
}
