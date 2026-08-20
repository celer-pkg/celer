// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Fusion
import QtQuick3D.Helpers
import QtQuick.Dialogs
import QtGraphs
import "."

Item {
    id: mainView
    width: 1280
    height: 1024


    Scatter3D {
        id: scatterGraph
        property int splineType: 0
        width: parent.width
        height: parent.height
        aspectRatio: 1
        axisX.min: -1.2
        axisX.max: 1.2
        axisZ.min: -1.2
        axisZ.max: 1.2
        theme: GraphsTheme {
            theme: GraphsTheme.Theme.QtGreen
            colorStyle: GraphsTheme.ColorStyle.Uniform
        }

        Spline3DSeries {
            id: splineSeries

            splineVisible: true
            splineTension: tensionSlider.value
            splineKnotting: knottingSlider.value
            splineLooping: looping.checked
            splineResolution: resolution.value
            itemSize: pointSizeSlider.value
            baseColor: "white"

            Component.onCompleted: splineGen.generateSpline(splineSeries, scatterGraph.splineType, points.value)
        }
    }


    RowLayout {
        id: settings
        anchors.top: parent.top
        anchors.left: parent.left

        ComboBox {
            id: splineTypeBox
            model: ["Circle","Helix", "Stitch curve"]
            onActivated:  {
                scatterGraph.splineType = currentIndex
                splineGen.generateSpline(splineSeries, scatterGraph.splineType, points.value)
            }
        }

        Button {
            id: liveToggle
            text: liveTimer.running? "Static" : "Live"
            onClicked: liveTimer.running = !liveTimer.running
        }

        ColumnLayout {
            Text {
                text: qsTr("Number of points")
                color: "white"
            }

            SpinBox {
                id: points
                from: 0
                value: 64
                to: 80
                editable: true
                onValueChanged: splineGen.generateSpline(splineSeries, scatterGraph.splineType, value);
            }
        }


        ColumnLayout {
            Text {
                text: qsTr("Point size")
                color: "white"
            }
            Slider {
                id: pointSizeSlider
                from: 0.001
                to: 0.1
                value: 0.001
            }
        }

        ColumnLayout {
            Text {
                text: qsTr("Spline color")
                color: "white"
            }

            Button{
                height: 25
                width: 25

                onClicked: splineCol.open()
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 5
                    color: splineSeries.splineColor
                }
            }

            ColorDialog {
                id: splineCol
                selectedColor: splineSeries.splineColor
                onAccepted: splineSeries.splineColor = selectedColor
            }
        }

        ColumnLayout {
            Text {
                text: qsTr("Spline resolution")
                color: "white"
            }

            SpinBox {
                id: resolution
                from: 2
                value: 10
                to: 30
                editable: true
            }
        }
    }

    RowLayout {
        id: splineSettings
        anchors.top: settings.bottom

        ColumnLayout {
            Text {
                id: tensionText
                text: qsTr("Tension")
                color: "white"
            }
            Slider {
                id: tensionSlider
                from: 0
                to: 1
                value: 0
                stepSize: 0.1
            }
        }

        ColumnLayout {
            Text {
                id: knottingText
                color: "white"
                text: qsTr("Knotting")
            }
            Slider {
                id: knottingSlider
                from: 0
                to: 1
                value: 0.5
                stepSize: 0.1
            }
        }
        ColumnLayout {
            Text {
                id: loopingText
                text: qsTr("Looping")
                color: "white"
            }
            CheckBox {
                id: looping
                checked: false
            }
        }
    }


    Timer {
        id: liveTimer
        interval: 1000 / 60
        repeat: true
        running: false
        onTriggered: splineGen.tickSpline(splineSeries)
    }
}
