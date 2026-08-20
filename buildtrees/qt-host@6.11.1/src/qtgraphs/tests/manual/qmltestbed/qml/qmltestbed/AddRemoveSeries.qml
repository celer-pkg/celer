// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtGraphs

Rectangle {
    anchors.fill: parent
    color: "#404040"

    Component {
        // Component with default bars.
        id: barComponent1
        BarSeries {
            labelsVisible: true
            labelsPrecision: 0
            BarSet { }
        }
    }
    Component {
        id: barComponent2
        // Component with custom bars.
        BarSeries {
            BarSet { }
            barDelegate: Item {
                property color barColor
                property real barValue
                anchors.alignWhenCentered: false
                Rectangle {
                    anchors.fill: parent
                    color: barColor
                    radius: width / 2
                    border.width: 2
                    border.color: "#ffffff"
                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: 24
                        color: "#ffffff"
                        text: barValue.toFixed(0)
                    }
                }
            }
        }
    }
    Component {
        id: lineComponent
        LineSeries {
            width: 2
            pointDelegate: Rectangle {
                property color pointColor
                width: 16
                height: 16
                color: "#202020"
                border.width: 2
                border.color: pointColor
                radius: width / 2
            }
            axisY: ValueAxis {
                max: 200
            }
        }
    }
    Component {
        id: splineComponent
        SplineSeries {
            width: 4
            pointDelegate: Rectangle {
                property color pointColor
                width: 16
                height: 16
                color: "#f0f0f0"
                border.width: 2
                border.color: pointColor
                radius: width / 2
            }
            axisX: ValueAxis {
                max: 2
            }
        }
    }
    Component {
        id: scatterComponent
        ScatterSeries {
            pointDelegate: Rectangle {
                property color pointColor
                width: 16
                height: 16
                color: pointColor
                border.width: 2
                border.color: "#f0f0f0"
                NumberAnimation on rotation {
                    from: 0
                    to: 360
                    loops: Animation.Infinite
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
    Component {
        id: areaComponent
        AreaSeries {
            borderColor: "#f0f000"
            borderWidth: 3
            upperSeries: LineSeries { }
        }
    }

    GraphsView {
        id: graphsView
        anchors.fill: parent
        anchors.margins: 50 * px
        anchors.rightMargin: settingsView.posX + 60 * px
        axisX: BarCategoryAxis {
            categories: ["Something", "Here"]
        }
        axisY: ValueAxis {
            id: yAxis
            max: 100
        }
    }

    SettingsView {
        id: settingsView
        Button {
            width: 200
            text: "Add series"
            onClicked: {
                let types = [barComponent1, barComponent2, lineComponent, splineComponent, scatterComponent, areaComponent]
                let type = Math.floor(Math.random() * types.length);
                let seriesComponent = types[type]
                let series = seriesComponent.createObject(graphsView)
                if (type == 0 || type == 1) {
                    // Bars
                    let set = series.at(0);
                    set.append(20 + 80 * Math.random())
                    set.append(20 + 80 * Math.random())
                    set.borderColor = "#404040"
                    set.color = Qt.rgba(Math.random(),
                                        Math.random(),
                                        Math.random(), 0.2)
                } else if (type == 5) {
                    // Area
                    let upperSeries = series.upperSeries
                    upperSeries.append(0, 20 + 80 * Math.random())
                    upperSeries.append(1, 20 + 80 * Math.random())
                    upperSeries.append(2, 20 + 80 * Math.random())
                    series.color = Qt.rgba(Math.random(),
                                           Math.random(),
                                           Math.random(), 0.5)
                } else {
                    // Other XYSeries
                    series.append(0, 20 + 80 * Math.random())
                    series.append(1, 20 + 80 * Math.random())
                    series.append(2, 20 + 80 * Math.random())
                    series.color = Qt.rgba(Math.random(),
                                           Math.random(),
                                           Math.random(), 0.5)
                }
                graphsView.addSeries(series)
            }
        }
        Button {
            width: 200
            text: "Remove first series"
            onClicked: {
                graphsView.removeSeries(0)
            }
        }
        Button {
            width: 200
            text: "Remove last series"
            onClicked: {
                graphsView.removeSeries(graphsView.seriesList.length - 1)
            }
        }
        Button {
            width: 200
            text: "Toggle first series visibility"
            onClicked: {
                if (graphsView.seriesList.length > 0) {
                    let series = graphsView.seriesList[0]
                    series.visible = !series.visible
                }
            }
        }
        Button {
            width: 200
            text: "Toggle last series visibility"
            onClicked: {
                if (graphsView.seriesList.length > 0) {
                    let series = graphsView.seriesList[graphsView.seriesList.length - 1]
                    series.visible = !series.visible
                }
            }
        }
    }
}
