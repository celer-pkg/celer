// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    id: base
    width: 800
    height: 600
    property list<string> graphsTests: ["GraphsLineTest.qml", "GraphsAreaTest.qml", "GraphsBarTest.qml", "GraphsSplineTest.qml"]
    property list<string> chartsTests: ["ChartsLineTest.qml", "ChartsAreaTest.qml", "ChartsBarTest.qml", "ChartsOpenGLLineTest.qml", "ChartsSplineTest.qml"]
    property list<string> tests: []
    property int testIndex: 0
    property int fps: 0
    property var results: ({})
    property string resultType: ""

    Component.onCompleted: {
        if (resultsIO.useCharts())
            tests = chartsTests;
        else
            tests = graphsTests;
        for (var i = 0; i < tests.length; i++) {
            results[tests[i]] = {};
            results[tests[i]].fps = [];
            results[tests[i]].counts = [];
        }
    }

    Column {
        anchors.horizontalCenter: base.horizontalCenter
        spacing: 2

        Button {
            text: "Run Benchmark"
            onClicked: {
                timer.running = true;
                loader.source = base.tests[base.testIndex];
            }
        }

        Button {
            text: "Show Results"
            onClicked: {
                base.resultType = ""
                loader.source = "Results.qml"
            }
        }

        Button {
            text: "Show Line Results"
            onClicked: {
                base.resultType = "Line"
                loader.source = "Results.qml"
            }
        }

        Button {
            text: "Show Area Results"
            onClicked: {
                base.resultType = "Area"
                loader.source = "Results.qml"
            }
        }

        Button {
            text: "Show Bar Results"
            onClicked: {
                base.resultType = "Bar"
                loader.source = "Results.qml"
            }
        }

        Button {
            text: "Show Spline Results"
            onClicked: {
                base.resultType = "Spline"
                loader.source = "Results.qml"
            }
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
    }

    Connections {
        target: loader.item

        function onMeasure(count) {
            var test = base.tests[base.testIndex];
            base.results[test].fps.push(base.fps);
            base.results[test].counts.push(count);
        }
    }

    /*Text {
        text: parent.fps
        font.family: "Helvetica"
        font.pointSize: 24
        color: "red"
    }*/

    Timer {
        id: timer
        interval: 32000; running: false; repeat: true
        onTriggered: {
            parent.testIndex++;
            if (parent.testIndex < parent.tests.length) {
                loader.source = parent.tests[parent.testIndex];
            } else {
                running = false;
                resultsIO.saveResults(JSON.stringify(base.results));
                loader.source = "Results.qml";
            }
        }
    }
}
