// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls.Basic
import QtQuick.Dialogs

Rectangle {
    id: mainview
    width: 800
    height: 600
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
                text: "Line 1 width"
                font.pixelSize: 12
                font.bold: true
                color: "#ffffff"
            }

            Slider {
                id: widthSlider1
                from: 0.1
                to: 10
                value: 2
            }
        }

        Column {
            Text {
                text: "Line 2 and 3 width"
                font.pixelSize: 12
                font.bold: true
                color: "#ffffff"
            }

            Slider {
                id: widthSlider2
                from: 0.1
                to: 10
                value: 2
            }
        }

        Row {
            id: seriesToolbar
            spacing: 10
            Button {
                text: "Theme1"
                onClicked: {
                    seriesTheme.theme = GraphsTheme.Theme.QtGreen;
                }
            }
            Button {
                text: "Theme2"
                onClicked: {
                    seriesTheme.theme = GraphsTheme.Theme.QtGreenNeon;
                }
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"
                font.pixelSize: 16
                text: "SELECTED: " + lineSeries1.selectedPoints
            }
        }

        Column {
            Button {
                text: "Reset zoom/pan"
                onClicked: {
                    xAxis.zoom = 1;
                    yAxis.zoom = 1;
                    xAxis.pan = 0;
                    yAxis.pan = 0;
                }
            }

            CheckBox {
                id: zoomAreaCheck
                text: "Zoom area enabled"

                contentItem: Text {
                    text: zoomAreaCheck.text
                    font: zoomAreaCheck.font
                    color: "#ffffff"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: zoomAreaCheck.indicator.width + zoomAreaCheck.spacing
                }
            }
        }
    }

    Rectangle {
        id: background
        anchors.fill: chartView
        color: "#202020"
        border.color: "#606060"
        border.width: 2
        radius: 10
    }

    SettingsView {
        id: settingsView
        CustomLabel {
            text: "X-coordinate: Min"
        }
        CustomSlider {
            id: sliderMinValueX
            sliderValue: xAxis.min
            fromValue: -40
            toValue: 0
            onSliderValueChanged: xAxis.min = sliderValue;
        }
        CustomLabel {
            text: "X-coordinate: Max"
        }
        CustomSlider {
            id: sliderSpeedValueX
            sliderValue: xAxis.max
            fromValue: 0
            toValue: 40
            onSliderValueChanged: xAxis.max = sliderValue;
        }
        CustomLabel {
            text: "Y-coordinate: Min"
        }
        CustomSlider {
            id: sliderMinValueY
            sliderValue: yAxis.min
            fromValue: -40
            toValue: 0
            onSliderValueChanged: yAxis.min = sliderValue;
        }
        CustomLabel {
            text: "Y-coordinate: Max"
        }
        CustomSlider {
            id: sliderSpeedValueY
            sliderValue: yAxis.max
            fromValue: 0
            toValue: 40
            onSliderValueChanged: yAxis.max = sliderValue;
        }
        CustomLabel {
            id: visualRangeMinXLabel
            text: qsTr("X Axis visualMin: %1").arg(Number(xAxis.visualMin).toLocaleString())
        }
        CustomLabel {
            id: visualRangeMaxXLabel
            text: qsTr("X Axis visualMax: %1").arg(Number(xAxis.visualMax).toLocaleString())
        }
        CustomLabel {
            id: visualRangeMinYLabel
            text: qsTr("Y Axis visualMin: %1").arg(Number(yAxis.visualMin).toLocaleString())
        }
        CustomLabel {
            id: visualRangeMaxYLabel
            text: qsTr("Y Axis visualMax: %1").arg(Number(yAxis.visualMax).toLocaleString())
        }
    }

    GraphsView {
        id: chartView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: toolbar.bottom
        anchors.margins: 10
        anchors.rightMargin: settingsView.posX + 20 * px
        zoomStyle: GraphsView.ZoomStyle.Center;
        panStyle: zoomAreaCheck.checked ? GraphsView.PanStyle.None : GraphsView.PanStyle.Drag
        zoomAreaEnabled: zoomAreaCheck.checked

        axisX: ValueAxis {
            id: xAxis
            max: 4
        }
        axisY: ValueAxis {
            id: yAxis
            max: 8
        }

        theme: GraphsTheme {
            id: seriesTheme
            theme: GraphsTheme.Theme.QtGreen
        }

        LineSeries {
            id: lineSeries1
            name: "First"
            width: widthSlider1.value
            selectable: true
            draggable: true
            pointDelegate: Image {
                property bool pointSelected: false
                source: "images/happy_box.png"
                width: pointSelected ? 96 : 64
                height: pointSelected ? 96 : 64
            }

            XYPoint { x: 0; y: 0 }
            XYPoint { x: 1.1; y: 2.1 }
            XYPoint { x: 1.9; y: 3.3 }
            XYPoint { x: 2.1; y: 2.1 }
            XYPoint { x: 2.9; y: 4.9 }
            XYPoint { x: 3.4; y: 3.0 }
            XYPoint { x: 4.0; y: 3.3 }
        }

        LineSeries {
            id: lineSeries2
            name: "Second"
            width: widthSlider2.value
            draggable: true
            hoverable: true
            pointDelegate: Item {
                property color pointColor
                property real pointValueX
                property real pointValueY
                property int pointIndex
                width: 16
                height: 16
                Rectangle {
                    anchors.fill: parent
                    color: "#202020"
                    border.width: 2
                    border.color: pointColor
                    radius: (pointIndex % 2 === 0) ? width / 2 : 2
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.top
                    color: "#ffffff"
                    font.pixelSize: 16
                    text: pointIndex + ": (" + pointValueX.toFixed(1) + ", " + pointValueY.toFixed(1) + ")"
                }
            }
            XYPoint { x: 0; y: 6.6 }
            XYPoint { x: 0.6; y: 4.1 }
            XYPoint { x: 1.5; y: 5.3 }
            XYPoint { x: 2.2; y: 7.1 }
            XYPoint { x: 3.3; y: 6.9 }
            XYPoint { x: 3.6; y: 5.0 }
            XYPoint { x: 4.0; y: 5.3 }

            onHoveredChanged: (enabled)=> {
                                  console.log("hoveredChanged:", enabled)
                                  console.log("isHovered:", lineSeries2.hovered)
                              }
        }

        LineSeries {
            id: lineSeries3
            name: "Third"
            width: widthSlider2.value
            XYPoint { x: 0; y: 2.6 }
            XYPoint { x: 0.2; y: 3.1 }
            XYPoint { x: 1.3; y: 6.3 }
            XYPoint { x: 2.4; y: 5.1 }
            XYPoint { x: 3.5; y: 6.9 }
            XYPoint { x: 3.6; y: 5.2 }
            XYPoint { x: 4.0; y: 3.3 }
        }
    }
}
