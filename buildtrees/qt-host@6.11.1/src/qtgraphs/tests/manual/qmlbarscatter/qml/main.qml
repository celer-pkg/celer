// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtGraphs
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts
import "."
import qmlbarscatter

Item {
    id: mainView
    width: 640
    height: 480
    visible: true

    Column {
        property double cWidth: 200

        id: col
        width: cWidth
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: 20

        Rectangle {
            color: "#202020"
            width: col.cWidth
            anchors.top: parent.top
            radius: 20

            Rectangle {
                color: "#202020"
                width: col.cWidth - 20
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }

            TabBar {
                id: tabBar

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.right: parent.right

                TabButton {
                    text: qsTr("Bar")
                }
                TabButton {
                    text: qsTr("Scatter")
                }
            }
        }

    }

    Rectangle {
        id: background
        anchors.fill: stack
        color: "#202020"
        radius: 10
    }

    StackLayout {
        id: stack

        anchors.top: parent.top
        anchors.left: col.right
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        currentIndex: tabBar.currentIndex
        anchors.margins: 10

        GraphsView {
            id: graph

            anchors.margins: 10
            theme: GraphsTheme {
                id: myTheme
                theme: GraphsTheme.QtGreenNeon
                axisXLabelFont.pixelSize: 20
                backgroundColor: "#202020"
            }

            axisX: BarCategoryAxis {
                categories: ["Reading 1", "Reading 2", "Reading 3", "Reading 4", "Reading 5"]
            }

            axisY: ValueAxis {
                max: 100
            }

            CustomBar {
                id: barSeries
            }
        }

        GraphsView {
            id: graph2

            anchors.margins: 10
            theme: GraphsTheme {
                id: myTheme2
                theme: GraphsTheme.QtGreen
                axisXLabelFont.pixelSize: 20
                backgroundColor: "#202020"
            }

            axisX: ValueAxis {
                id: xAxis
                max: 5
            }

            axisY: ValueAxis {
                id: yAxis
                max: 100
            }

            CustomScatter {
                id: scatterSeries

                pointDelegate: Image {
                    property bool selected: false
                    source: "qrc:/images/img.png"
                    width: 64
                    height: 64
                }

                XYPoint { x: 10; y: 50 }
            }
        }

    }
}
