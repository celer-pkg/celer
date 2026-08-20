// Copyright (C) 2023 The Qt Company Ltd.
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
        }
        axisY: ValueAxis {
            id: axisY
            max: 10
            subTickCount: 9
        }
        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.QtGreen
            axisXLabelFont.pixelSize: 20
        }
        BarSeries {
            id: barSeries
            selectable: true
            hoverable: true

            BarSet { id: set1; label: "Axel"; values: [1, 2, 3, 4]; selectedColor: "red" }
            BarSet { id: set2; label: "Frank"; values: [8, 2, 6, 0] }
            BarSet { id: set3; label: "James"; values: [4+3*Math.sin(fA.elapsedTime), 5+3*Math.sin(fA.elapsedTime), 2, 3] }
            FrameAnimation {
                id: fA
                running: true
            }
            onHoveredChanged: (enabled)=> {
                                  console.log("hoveredChanged:", enabled)
                                  console.log("isHovered:", hovered)
                              }
        }
    }
    Text {
        id: text1
        y: 10
        x: 80
        color: "#ffffff"
        font.pixelSize: 20
        text: "Set 1, selected bars: " + set1.selectedBars + "\n" +
              "Set 2, selected bars: " + set2.selectedBars + "\n" +
              "Set 3, selected bars: " + set3.selectedBars;
    }

    SettingsView {
        id: settingsView
        Item {
            width: 260
            height: 10
        }
        Button {
            width: 250
            text: "Select All"
            onClicked: barSeries.selectAll();
        }
        Button {
            width: 250
            text: "Deselect All"
            onClicked: barSeries.deselectAll();
        }
        Button {
            width: 250
            text: "Select Set 1"
            onClicked: set1.selectAllBars();
        }
        Button {
            width: 250
            text: "Deselect Set 1"
            onClicked: set1.deselectAllBars();
        }
        Button {
            width: 250
            text: "Select Set 3, bar 0"
            onClicked: set3.selectBar(0);
        }
        Button {
            width: 250
            text: "Select Set 3, bars 1 and 2"
            onClicked: set3.selectBars([1, 2]);
        }
        Button {
            width: 250
            text: "Toggle Set 3, bars 1 and 2"
            onClicked: set3.toggleSelection([1, 2]);
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
    }
}
