// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls.Basic
import TestbedExample

Rectangle {
    anchors.fill: parent
    color: "#404040"

    Row {
        id: toolbar
        anchors.top: parent.top
        anchors.margins: 10
        anchors.left: parent.left
        anchors.leftMargin: 60
        spacing: 10

        Column {
            Text {
                text: "Amplitude"
                font.pixelSize: 12
                font.bold: true
                color: "#ffffff"
            }

            Slider {
                id: ampSlider
                from: 0
                to: 1
                value: 1
            }

            Text {
                text: ampSlider.value.toFixed(2)
                font.pixelSize: 12
                font.bold: true
                color: "#ffffff"
            }
        }

        Column {
            Text {
                text: "Frequency"
                font.pixelSize: 12
                font.bold: true
                color: "#ffffff"
            }

            Slider {
                id: freqSlider
                from: 1
                to: 60
                value: 10
            }

            Text {
                text: freqSlider.value.toFixed(2)
                font.pixelSize: 12
                font.bold: true
                color: "#ffffff"
            }
        }

        Column {
            Text {
                text: "Phase"
                font.pixelSize: 12
                font.bold: true
                color: "#ffffff"
            }

            Slider {
                id: phaseSlider
                from: 0
                to: 4
                value: 0
            }

            Text {
                text: phaseSlider.value.toFixed(2)
                font.pixelSize: 12
                font.bold: true
                color: "#ffffff"
            }
        }
    }

    GraphsView {
        id: chartView
        anchors.fill: parent
        anchors.margins: 20 * px
        anchors.topMargin: 80 * px
        axisX: ValueAxis {
            max: 4
        }
        axisY: ValueAxis {
            min: -1
            max: 1
        }
        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.QtGreen
            axisXLabelFont.pixelSize: 20
        }
        CustomLineSeries {
            id: lineSeries
            amplitude: ampSlider.value
            frequency: freqSlider.value
            phase: phaseSlider.value
        }
    }
}
