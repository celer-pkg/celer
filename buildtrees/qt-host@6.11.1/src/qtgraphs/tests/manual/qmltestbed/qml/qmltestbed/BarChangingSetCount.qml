// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtGraphs
import QtQuick.Controls.Basic
import TestbedExample

Rectangle {
    id: mainView
    color: "#404040"

    FontLoader {
        id: customFont
        source: "images/Sevillana-Regular.ttf"
    }

    Row {
        id: legendRow
        anchors.margins: 10
        anchors.horizontalCenter: mainView.horizontalCenter
        anchors.bottom: chartView.top
        spacing: 10
        Repeater {
            model: mySeries.legendData.length
            Rectangle {
                id: legend1
                height: 20
                width: 40
                color: mySeries.legendData[index].color
                Text {
                    id: text1
                    text: mySeries.legendData[index].label
                }
            }
        }
    }

    Rectangle {
        id: background
        anchors.left: setControls.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.topMargin: 100
        anchors.margins: 10
        color: "#202020"
        border.color: "#606060"
        border.width: 2
        radius: 10
    }

    GraphsView {
        id: chartView
        anchors.fill: background
        anchors.leftMargin: 30

        axisX: BarCategoryAxis {
            categories: ["2007", "2008", "2009", "2010", "2011", "2012"]
        }
        axisY: ValueAxis {
            subTickCount: 4
            max: 20
        }
        theme: GraphsTheme {
            backgroundVisible: false
            theme: GraphsTheme.Theme.MixSeries
        }
        BarSeries {
            id: mySeries
            barWidth: 1.0 - (1.0 / (1 + count))
        }
    }
    Column {
        id: setControls
        anchors.left: mainView.left
        anchors.leftMargin: 10
        anchors.verticalCenter: chartView.verticalCenter
        spacing: 2

        BarSetGenerator {
            id: generator
        }

        Button {
            id: removeBtn
            text: qsTr("remove set")
            onClicked: {
                mySeries.remove(mySeries.count - 1)
            }
        }
        Button {
            id: addBtn
            text: qsTr("add set")
            onClicked: {
                var s = generator.createNewBarSet()
                mySeries.append(s)
            }
        }
    }
}
