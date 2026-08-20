// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls

Rectangle {
    id: mainView
    width: 800
    height: 600
    color: "#4F4040"

    GraphsView {
        property int xMax: xAxis.max
        property int yMax: yAxis.max

        id: chartView
        anchors.left: parent.left
        anchors.right: rcolumn.left
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 10

        axisY: ValueAxis {
            id: yAxis
            max: 5
        }

        axisX: ValueAxis {
            id: xAxis
            tickInterval: 1
            max: 6
            min: 0
        }

        SplineSeries {
            id: spline
            name: "Spline"
            width: 4
            color: "#d06040"

            GraphTransition {
                GraphPointAnimation { duration: 1000; easingCurve.type: Easing.OutCubic }
                SplineControlAnimation { duration: 1000; easingCurve.type: Easing.OutCubic }
            }

            XYPoint { x: 0.0; y: 0.0 }
        }

        LineSeries {
            id: line
            name: "Line"
            width: 8

            GraphTransition {
                GraphPointAnimation { duration: 1000; easingCurve.type: Easing.OutCubic }
            }

            XYPoint { x: 0.0; y: 0.0 }
        }
    }

    Column {
        property int splineValue: 0
        property int lineValue: 0

        id: rcolumn
        spacing: 5
        anchors.margins: 30
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 300

        Text {
            text: "Spline:"
            font.pixelSize: 24
            color: "#ffffff"
        }

        RoundButton {
            property alias v: rcolumn.splineValue
            text: "Add Point"
            anchors.left: parent.left
            anchors.right: parent.right

            onClicked: {
                if (v >= chartView.xMax)
                    return

                spline.append(++v, Math.random() * chartView.yMax)
            }
        }

        SpinBox {
            id: sbox
            to: chartView.xMax
            anchors.left: parent.left
            anchors.right: parent.right
        }

        RoundButton {
            property alias v: rcolumn.splineValue
            text: "Replace Point"
            anchors.left: parent.left
            anchors.right: parent.right

            onClicked: {
                sbox.value = Math.min(v, sbox.value)

                spline.replace(sbox.value, sbox.value, Math.random() * chartView.yMax)
            }
        }

        RoundButton {
            property alias v: rcolumn.splineValue
            text: "Remove Point"
            anchors.left: parent.left
            anchors.right: parent.right
            enabled: true

            onClicked: {
                if (v == 0)
                    return

                spline.remove(v--)
            }
        }

        Item {
            height: 50
            width: 200
        }

        Text {
            text: "Line:"
            font.pixelSize: 24
            color: "#ffffff"
        }

        RoundButton {
            property alias v: rcolumn.lineValue
            text: "Add Point"
            anchors.left: parent.left
            anchors.right: parent.right

            onClicked: {
                if (v >= chartView.xMax)
                    return

                line.append(++v, Math.random() * chartView.yMax)
            }
        }

        SpinBox {
            id: lbox
            to: chartView.xMax
            anchors.left: parent.left
            anchors.right: parent.right
        }

        RoundButton {
            property alias v: rcolumn.lineValue
            text: "Replace Point"
            anchors.left: parent.left
            anchors.right: parent.right

            onClicked: {
                lbox.value = Math.min(v, lbox.value)

                line.replace(lbox.value, lbox.value, Math.random() * chartView.yMax)
            }
        }

        RoundButton {
            property alias v: rcolumn.lineValue
            text: "Remove Point"
            anchors.left: parent.left
            anchors.right: parent.right
            enabled: true

            onClicked: {
                if (v == 0)
                    return

                line.remove(v--)
            }
        }
    }
}
