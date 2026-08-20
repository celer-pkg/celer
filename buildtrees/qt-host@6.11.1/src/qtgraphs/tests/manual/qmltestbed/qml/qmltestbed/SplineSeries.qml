// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: mainView
    width: 800
    height: 600
    color: "#4F4040"

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            spacing: 20

            Text {
                id: label
                color: "white"
                text: "Default"
            }
            Button {
                Layout.margins: 10
                Layout.bottomMargin: 0
                text: "Switch Fitting"
                onClicked: {
                    spline.optimized = !spline.optimized;
                    label.text = spline.optimized ? "Catmull-Rom" : "Default";
                }
            }
        }

        GraphsView {
            id: chartView
            Layout.margins: 10
            Layout.fillHeight: true
            Layout.fillWidth: true

            axisX: ValueAxis {
                id: xAxis
                max: 4.1
            }
            axisY: ValueAxis {
                id: yAxis
                max: 3.2
            }

            SplineSeries {
                id: spline
                name: "Spline"
                draggable: true
                width: 2

                pointDelegate: Rectangle {
                    property color pointColor
                    color: pointColor
                    width: 16
                    height: 16
                }

                XYPoint { x: 0; y: 0.0 }
                XYPoint { x: 1.1; y: 1.1 }
                XYPoint { x: 1.9; y: 2.4 }
                XYPoint { x: 1.5; y: 1.8 }
                XYPoint { x: 2.9; y: 2.6 }
                XYPoint { x: 3.4; y: 2.3 }
                XYPoint { x: 4.1; y: 3.1 }
            }
        }
    }
}
