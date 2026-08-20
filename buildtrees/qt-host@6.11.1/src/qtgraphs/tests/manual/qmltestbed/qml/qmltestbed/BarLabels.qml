// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls.Basic

Rectangle {
    anchors.fill: parent
    color: "#404040"

    GraphsView {
        id: chartView
        anchors.fill: parent
        anchors.margins: 20 * px
        anchors.topMargin: 80 * px
        anchors.rightMargin: settingsView.posX + 20 * px
        axisX: BarCategoryAxis {
            categories: ["2023", "2024", "2025", "2026"]
            subGridVisible: false
            labelDelegate: Item {
                id: comp
                property string text
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
                    color: "#202020"
                    border.color: mouseArea.containsMouse ?  "#f0f020" : "#d0d0d0"
                    border.width: 2
                    radius: 5
                }
                Text {
                    anchors.centerIn: parent
                    color: "#ffffff"
                    font.pixelSize: 20
                    text: comp.text
                    rotation: chartView.orientation == Qt.Horizontal ? -90 : 0
                }
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }
        }
        axisY: ValueAxis {
            id: axisY
            max: 10
            subTickCount: 9
            labelDelegate: Text {
                style: Text.Outline
                styleColor: "#800000"
                color: "#ffffff"
            }
        }
        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.QtGreen
            backgroundVisible: false
            axisXLabelFont.pixelSize: 20
        }
        BarSeries {
            id: barSeries
            selectable: true
            labelsVisible: checkBoxLabelsVisible.checked
            labelsPrecision: barsDecimalsSlider.sliderValue
            labelsMargin: barsMarginSlider.sliderValue
            labelsAngle: barsAngleSlider.sliderValue
            BarSet { id: set2; label: "Frank"; labelColor: "#000000"; values: [8, 2, 6, 0] }
            BarSet { id: set3; label: "James"; labelColor: "#ff00ff"; values: [4+3*Math.sin(fA.elapsedTime), 5+3*Math.sin(fA.elapsedTime), 2, 3] }
            FrameAnimation {
                id: fA
                running: true
            }
        }
    }
    SettingsView {
        id: settingsView
        Item {
            width: 260
            height: 10
        }
        CustomCheckBox {
            id: checkBoxLabelsVisible
            text: "Labels visible"
            checked: true
        }
        Button {
            width: 250
            text: "Grouped/Stacked/Percent bars"
            onClicked: {
                if (barSeries.barsType == BarSeries.BarsType.StackedPercent) {
                    barSeries.barsType = BarSeries.BarsType.Groups
                    axisY.max = 10
                } else if (barSeries.barsType == BarSeries.BarsType.Stacked) {
                    barSeries.barsType = BarSeries.BarsType.StackedPercent
                    axisY.max = 100
                } else {
                    barSeries.barsType = BarSeries.BarsType.Stacked
                    axisY.max = 25
                }
            }
        }
        Button {
            width: 250
            text: "Vertical/Horizontal bars"
            onClicked: {
                if (chartView.orientation === Qt.Vertical)
                    chartView.orientation = Qt.Horizontal
                else
                    chartView.orientation = Qt.Vertical
            }
        }
        Button {
            width: 250
            text: "Labels position"
            onClicked: {
                if (barSeries.labelsPosition === BarSeries.LabelsPosition.Center)
                    barSeries.labelsPosition = BarSeries.LabelsPosition.InsideEnd
                else if (barSeries.labelsPosition === BarSeries.LabelsPosition.InsideEnd)
                    barSeries.labelsPosition = BarSeries.LabelsPosition.InsideBase
                else if (barSeries.labelsPosition === BarSeries.LabelsPosition.InsideBase)
                    barSeries.labelsPosition = BarSeries.LabelsPosition.OutsideEnd
                else
                    barSeries.labelsPosition = BarSeries.LabelsPosition.Center
            }
        }
        Button {
            width: 250
            text: "Labels format on/off"
            onClicked: {
                if (barSeries.labelsFormat === "")
                    barSeries.labelsFormat = "time\n@value s"
                else
                    barSeries.labelsFormat = ""
            }
        }
        CustomLabel {
            text: "Labels margin"
        }
        CustomSlider {
            id: barsMarginSlider
            sliderValue: 10
            fromValue: 0
            toValue: 40
        }
        CustomLabel {
            text: "Labels angle"
        }
        CustomSlider {
            id: barsAngleSlider
            sliderValue: 0
            fromValue: -90
            toValue: 90
        }
        CustomLabel {
            text: "Labels decimals"
        }
        CustomSlider {
            id: barsDecimalsSlider
            sliderValue: 1
            sliderStepSize: 1
            fromValue: 0
            toValue: 4
        }
    }
}
