// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtDataVisualization
import "."

Item {
    id: tests

    property alias currentFps: fpsText.fps
    property alias dataPoints: dataPointText.dataPoints
    property alias buttonVisible: testButton.visible
    property real averageFps: 0

    signal onTestFinished

    property list<real> fpsCounts: []
    Component.onCompleted: {
        dataGenerator.onMessage.connect(addLine)
    }

    function addLine(line) {
        logModel.append({'logText':line});
    }


    function startTest() {
        // logModel.clear()
        fpsCounts = []
        dataGenerator.writeLine(" ")
        switch (tabBar.currentIndex) {
        case 0:
            dataGenerator.writeLine("Started surface test with configuration:")
            break
        case 1:
            dataGenerator.writeLine("Started scatter test with configuration:")
            break
        case 2:
            dataGenerator.writeLine("Started bars test with configuration:")
            break
        default:
            break
        }

        if (tabBar.currentIndex  === 0) {
            dataGenerator.writeLine("Shadow Quality: " + surfaceGraph.shadowQuality)
            dataGenerator.writeLine("MSAA samples: "
                                + surfaceGraph.msaaSamples)
        } else if (tabBar.currentIndex === 1) {
            dataGenerator.writeLine("Shadow Quality: " + scatterGraph.shadowQuality)
            var optimizationString = scatterGraph.optimizationHint? "Static" : "Default"
            dataGenerator.writeLine("Optimization: " + optimizationString)
            dataGenerator.writeLine("MSAA samples: "
                                + scatterGraph.msaaSamples)
        } else {
            dataGenerator.writeLine("Shadow Quality: " + scatterGraph.shadowQuality)
            optimizationString = barGraph.optimizationHint? "Static" : "Default"
            dataGenerator.writeLine("Optimization: " + optimizationString)
            dataGenerator.writeLine("MSAA samples: "
                                + barGraph.msaaSamples)

        }

        testTimer.start()
    }

    Button {
        id: testButton
        text: "Test current"
        onClicked: startTest()
        height: 50
        width: parent.width - 20
        anchors.horizontalCenter: parent.horizontalCenter
    }


    ColumnLayout {
        id: statsPanel
        anchors.top: testButton.bottom
        anchors.topMargin: 20
        width: parent.width
        Text {
            id: statsBanner
            text: "Statistics"
            font.bold: true
            font.pixelSize: 16
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            id: fpsText
            property real fps: 0
            text: qsTr("FPS: %1").arg(fps)
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter

        }

        Text {
            id: dataPointText
            property int dataPoints: 0
            text : qsTr("Data Points: %1").arg(dataPoints)
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            id: logBanner
            text: "Log"
            font.bold: true
            font.pixelSize: 16
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }


        Rectangle {
            id: logBackground
            Layout.fillWidth: true
            Layout.preferredHeight: 170
            Layout.margins: 10
            color: "forestgreen"
            ListView {
                id: logView
                anchors.fill: parent
                highlightFollowsCurrentItem: true
                clip: true
                delegate: Text {
                    text: logText
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    width: logView.width
                    wrapMode: Text.Wrap
                }

                model: ListModel {
                    id: logModel
                }

                onCountChanged: {
                    logView.currentIndex = count - 1
                }
            }
        }
    }

    Timer {
        id: testTimer
        interval: 1000
        repeat: true
        onTriggered: {
            var fps = 0
            if (tabBar.currentIndex === 0)
                fps = surfaceGraph.currentFps
            else if (tabBar.currentIndex === 1)
                fps = scatterGraph.currentFps
            else
                fps = barGraph.currentFps

            if (fps != -1) {
                fpsCounts.push(fps)
                dataGenerator.writeLine("FPS: " + fps)
            }
            else {
                dataGenerator.writeLine("Invalid fps reading")
            }

            if (fpsCounts.length >= 5) {
                var sum = 0
                fpsCounts.forEach((element) => sum+=element);
                averageFps = sum / fpsCounts.length
                dataGenerator.writeLine("Average FPS: " + averageFps)
                testTimer.stop()
                onTestFinished()
            }
        }
    }
}
