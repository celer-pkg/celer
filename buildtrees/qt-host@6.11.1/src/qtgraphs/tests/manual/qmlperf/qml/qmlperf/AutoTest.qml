// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtGraphs
import "."

Item {

    height: 150

    property list<string> graphTypes: ["Surface", "Scatter", "Bars"]
    property int currentGraph: 0

    property list<int>dataPoints: [10, 100, 300, 500]
    property int currentPoints: 0

    property list<string> optimization: ["Default", "Legacy"]
    property int currentOptimization: 0

    property list<int> sampleCounts: [0,2,4,8]
    property int currentSamples: 0


    property int shadowQuality

    property bool finished: true

    property int initTime: 0

    function initAutoTest() {
        //go through each graph iterating over all the variables
        liveDataCB.checked = true
        rotateCB.checked = true

        tests.onTestFinished.connect(test)
        dataGenerator.onCaptureInit.connect(setInitTime)

        finished = false

        currentGraph = 0
        currentPoints = 0
        currentOptimization = 0
        currentSamples= 0
        shadowQuality = 0

        setParameters()
        tests.startTest()
    }

    function test() {
        //write previous test results
        var averageFps = tests.averageFps
        // graph type, num points, optimization, msaa, init time, averagefps
        var csvLine = graphTypes[currentGraph] + ","
                + (dataPoints[currentPoints] * dataPoints[currentPoints])+ ","
                + optimization[currentOptimization] + ","
                + sampleCounts[currentSamples] + ","
                + shadowQuality + ","
                + initTime + ","
                + averageFps

        dataGenerator.writeCSV(csvLine)
        increment()
        setParameters()
        if (!finished) {
            tests.startTest()
        } else {
            tests.onTestFinished.disconnect(test)
            dataGenerator.onCaptureInit.disconnect(setInitTime)
        }
    }

    function increment() {
        if (varyShadow.checked) {
            if (shadowQuality < 6) {
                shadowQuality++
                return
            }
            shadowQuality = 0
        }

        if (varySamples.checked) {
            if (currentSamples < sampleCounts.length -1) {
                currentSamples ++
                return
            }
            currentSamples = 0
        }

        if (varyOptimization.checked) {
            if (currentOptimization < optimization.length -1
                    && tabBar.currentIndex !== 0) {
                currentOptimization++
                return
            }
            currentOptimization = 0
        }

        if (varyPoints.checked) {
            if (currentPoints < dataPoints.length -1) {
                currentPoints ++
                return
            }
            currentPoints = 0
        }

        if (varyGraphs.checked) {
            if (currentGraph < graphTypes.length - 1) {
                currentGraph++
                console.log("Switching to " + graphTypes[currentGraph])
                return
            }
            currentGraph = 0
        }

        dataGenerator.writeLine("Finished all tests!")
        finished = true
    }


    function setParameters() {
        if (varyShadow.checked) {
            surfaceGraph.shadowQuality = shadowQuality
            scatterGraph.shadowQuality = shadowQuality
            barGraph.shadowQuality = shadowQuality
        }

        if (varySamples.checked) {
            surfaceGraph.msaaSamples = sampleCounts[currentSamples]
            scatterGraph.msaaSamples = sampleCounts[currentSamples]
            barGraph.msaaSamples = sampleCounts[currentSamples]
        }

        if (varyOptimization.checked) {
            if (optimization[currentOptimization] === "Legacy") {
                scatterGraph.optimizationHint = Graphs3D.OptimizationHint.Default
                barGraph.optimizationHint = Graphs3D.OptimizationHint.Default
            } else {
                scatterGraph.optimizationHint = Graphs3D.OptimizationHint.Legacy
                barGraph.optimizationHint = Graphs3D.OptimizationHint.Legacy
            }
        }

        if (varyGraphs.checked)
            tabBar.setCurrentIndex(currentGraph)

        if (varyPoints.checked) {
            if (tabBar.currentIndex === 0)
                dataGenerator.generateSurfaceData(surfaceSeries, dataPoints[currentPoints])
            else if (tabBar.currentIndex === 1)
                dataGenerator.generateScatterData(scatterSeries, dataPoints[currentPoints])
            else
                dataGenerator.generateBarData(barSeries, dataPoints[currentPoints])
        }
    }

    function setInitTime(nsecs) {
        initTime = nsecs
    }

    Button {
        id: autoButton
        text: finished? "Auto Test" : "End test"
        width: parent.width - 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        height: 50
        onClicked: {
            if (finished) {
                dataGenerator.writeLine("Testing configurations...")
                initAutoTest()
            } else {
                finished = true
            }

        }
    }

    GridLayout {
        id: autoTestParams
        anchors.top: autoButton.bottom
        anchors.topMargin: 10
        width: parent.width - 50
        enabled: finished
        anchors.horizontalCenter: parent.Center
        height: 50
        columns: 2
        CheckBox {
            id: varyGraphs
            text: qsTr("Vary graphs")
            Layout.alignment: Qt.AlignCenter
            checked: true
        }
        CheckBox {
            id: varyPoints
            text: qsTr("Vary points")
            Layout.alignment: Qt.AlignCenter
            checked: true
        }
        CheckBox {
            id: varyOptimization
            text: qsTr("Vary optimization")
            Layout.alignment: Qt.AlignCenter
        }
        CheckBox {
            id: varySamples
            text: qsTr("Vary MSAA ")
            Layout.alignment: Qt.AlignCenter
        }
        CheckBox {
            id: varyShadow
            text: qsTr("Vary Shadow ")
            Layout.alignment: Qt.AlignCenter
        }
    }
}
