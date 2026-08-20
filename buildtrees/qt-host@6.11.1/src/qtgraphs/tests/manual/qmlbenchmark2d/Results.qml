// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs

Rectangle {
    signal measure(int count)
    id: base
    width: 800
    height: 600
    color: myTheme.backgroundColor

    property list<string> categories

    Component {
        id: lineComponent
        LineSeries {
        }
    }

    GraphsView {
        id: graph
        antialiasing: true
        anchors.fill: parent
        anchors.leftMargin: 20
        marginLeft: 0

        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            // seriesColors: ["#ffaaaa", "#aaffaa", "#aaaaff", "#ff0000", "#ffffaa", "#aaffff", "#ffaaff"]
            seriesColors: ["#ff0000", "#00ff00", "#0000ff", "#ffffff", "#550000", "#ffff00", "#00ffff", "#ff00ff", "#555500"]
        }

        axisX: BarCategoryAxis {
            id: xAxis
            categories: base.categories
            titleText: "Data Count"
            titleColor: "white"
        }

        axisY: ValueAxis {
            min: 0
            max: 80
            tickInterval: 10
            titleText: "FPS"
            titleColor: "white"
        }

        Component.onCompleted: {
            var chartsString = resultsIO.loadChartsResults();
            var graphsString = resultsIO.loadGraphsResults();

            var resultsCharts = {};
            var resultsGraphs = {};

            if (chartsString.length > 0)
                resultsCharts = JSON.parse(chartsString);

            if (graphsString.length > 0)
                resultsGraphs = JSON.parse(graphsString);

            var results = Object.assign(resultsCharts, resultsGraphs);

            var resultType = base.parent.parent.resultType;
            if (resultType !== "") {
                Object.keys(results).forEach((key) => key.includes(resultType) || delete results[key]);
            }

            for (var test in results) {
                if (Object.prototype.hasOwnProperty.call(results, test)) {
                    var counts = results[test].counts;

                    for (var i = 0; i < counts.length; i++) {
                        var countString = counts[i].toString();

                        if (base.categories.indexOf(countString) === -1)
                            base.categories.push(countString);
                    }
                }
            }

            base.categories.sort((a, b) => { return +a - +b; });

            for (var test in results) {
                if (Object.prototype.hasOwnProperty.call(results, test)) {
                    var counts = results[test].counts;

                    if (counts.length > 0) {
                        var countOfCounts = {};
                        for (var i = 0; i < counts.length; i++) {
                            var countString = counts[i].toString();
                            if (!(countString in countOfCounts))
                                countOfCounts[countString] = 0;

                            countOfCounts[countString]++;
                        }

                        let series = lineComponent.createObject(graph);
                        series.name = test;

                        var fps = results[test].fps;

                        var countIndex = 0;
                        var lastCount = counts[0].toString();
                        for (var i = 0; i < counts.length; i++) {
                            var countString = counts[i].toString();

                            if (lastCount !== countString) {
                                countIndex = 0;
                                lastCount = countString;
                            }

                            var maxIndex = countOfCounts[countString];
                            var x = base.categories.indexOf(countString) + (countIndex / maxIndex);
                            var y = fps[i];

                            if (x > 0)
                                series.append(x, y);

                            countIndex++;
                        }

                        addSeries(series);

                        repeater.model = graph.seriesList.length
                    }
                }
            }
        }
    }

    Column {
        anchors.right: graph.right
        anchors.top: graph.top
        spacing: 2
        Repeater {
            id: repeater
            model: 0
            Rectangle {
                required property int index
                height: 20
                width: text.width
                color: {
                    if (graph.seriesList[index].legendData.length > 0)
                        return graph.seriesList[index].legendData[0].color;
                    else
                        return "black";
                }
                Text {
                    id: text
                    text:
                        if (graph.seriesList[index].legendData.length > 0)
                            return graph.seriesList[index].legendData[0].label;
                        else
                            return "";
                }
            }
        }
    }
}
