// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls.Basic

Rectangle {
    anchors.fill: parent
    color: "#404040"

    property var queriedPoint
    property bool selecting: true

    GraphsView {
        id: graphView
        anchors.fill: parent
        anchors.margins: 20 * px
        anchors.topMargin: 80 * px
        anchors.rightMargin: settingsView.posX + 20 * px
        axisX: ValueAxis {
            id: axisX
            max: 4
            subTickCount: 9
        }
        axisY: ValueAxis {
            id: axisY
            max: 6
            subTickCount: 9
        }
        LineSeries {
            id: lineSeries
            selectable: true
            XYPoint { x: 0.0; y: 2.5 }
            XYPoint { x: 1.0; y: 3.3 }
            XYPoint { x: 2.0; y: 2.1 }
            XYPoint { x: 3.0; y: 4.9 }
            XYPoint { x: 4.0; y: 3.0 }
            pointDelegate: Item {
                property color pointColor
                property real pointValueX
                property real pointValueY
                property bool pointSelected
                width: 20
                height: 20
                Rectangle {
                    anchors.fill: parent
                    color: pointSelected ? "#f08060" : "#202020"
                    border.width: 2
                    border.color: pointColor
                    radius: width / 2
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.top
                    color: "#ffffff"
                    font.pixelSize: 16
                    text: "(" + pointValueX.toFixed(1) + ", " + pointValueY.toFixed(1) + ")"
                }
            }
        }
    }

    Text {
        id: text1
        y: 20
        x: 80
        color: "#ffffff"
        text: selecting ? "Selected points: " + lineSeries.selectedPoints
                        : "Queried position: " + queriedPoint
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
            onClicked: lineSeries.selectAllPoints();
        }
        Button {
            width: 250
            text: "Deselect All"
            onClicked: lineSeries.deselectAllPoints();
        }
        Button {
            width: 250
            text: "Select points 0 and 3"
            onClicked: {
                lineSeries.selectPoint(0);
                lineSeries.setPointSelected(3, true);
            }
        }
        Button {
            width: 250
            text: "Select points 1 and 2"
            onClicked: lineSeries.selectPoints([1, 2]);
        }
        Button {
            width: 250
            text: "Deselect points 1 and 2"
            onClicked: lineSeries.deselectPoints([1, 2]);
        }
        Button {
            width: 250
            text: "Toggle points 1 and 2"
            onClicked: lineSeries.toggleSelection([1, 2]);
        }
        Button {
            width: 250
            text: "Toggle data point query and selection"
            onClicked: selecting = !selecting
        }
    }

    MouseArea {
        enabled: !selecting
        anchors.fill: graphView
        onClicked: {
            queriedPoint = lineSeries.dataPointCoordinatesAt(mouse.x - graphView.plotArea.x,
                                                             mouse.y - graphView.plotArea.y)
        }
    }
}
