// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtGraphs
import QtQuick.Controls
import TestbedExample

Rectangle {
    id: background
    anchors.fill: parent
    color: "#404040"

    function addMapping() {
        pieModel.clearMapping()
        if (pieModelMapper.orientation === Qt.Vertical) {
            for (var i = 0; i < pieSeries.legendData.length; i++) {
                var cf = pieModelMapper.labelsSection
                var cl = 2
                var rf = pieModelMapper.first + i
                var rl = 1
                var c = pieSeries.legendData[i].color
                pieModel.addMapping(c, cf, rf, cl, rl)
            }
        }
    }

    function updateMapping() {
        pieModel.startAddMapping()
        addMapping()
        pieModel.endAddMapping()
    }

    function showLabels() {
        for (var i = 0; i < pieSeries.count; ++i)
            pieSeries.at(i).labelVisible = true
    }

    Item {
        id: tableViewItem

        property alias tableView: tableView
        anchors.top: title.bottom
        anchors.left: background.left
        anchors.bottom: background.bottom
        implicitWidth: 150
        implicitHeight: 300

        TableView {
            id: tableView
            anchors.top: tableViewItem.top
            anchors.left: tableViewItem.left

            implicitWidth: tableViewItem.implicitWidth
            implicitHeight: tableViewItem.implicitHeight

            reuseItems: false
            clip: true
            model: PieModelMapperModel {
                id: pieModel
            }
            delegate: Rectangle {
                id: delegateRoot
                required property string display
                required property color background
                implicitHeight: 30

                implicitWidth: tableView.width / 2
                border.width: 1
                color: background
                Text {
                    id: delegateText
                    text: delegateRoot.display
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    anchors.leftMargin: 4
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
            }
        }
    }

    Row {
        id: controlRow
        anchors.top: background.top
        Text {
            id: firstRowLabel
            text: qsTr("First row")
            anchors.verticalCenter: firstRowSlider.verticalCenter
        }
        Text {
            id: firstRowValue
            text: qsTr("1")
            anchors.verticalCenter: firstRowSlider.verticalCenter
        }

        Slider {
            id: firstRowSlider
            from: 0
            to: 5
            value: 1
            stepSize: 1.0
            onMoved: () => {
                         pieModelMapper.first = firstRowSlider.value
                         firstRowValue.text = firstRowSlider.value
                         updateMapping()
                         showLabels()
                     }
        }
        Text {
            id: rowCountLabel
            text: qsTr("Row count")
            anchors.verticalCenter: rowCountSlider.verticalCenter
        }

        Text {
            id: rowCountValue
            text: qsTr("4")
            anchors.verticalCenter: rowCountSlider.verticalCenter
        }

        Slider {
            id: rowCountSlider
            from: 1
            to: 6
            value: 4
            stepSize: 1.0
            onMoved: () => {
                         pieModelMapper.count = rowCountSlider.value
                         rowCountValue.text = rowCountSlider.value
                         updateMapping()
                         showLabels()
                     }
        }
    }

    Text {
        id: title
        text: "Simple Pie Graph"
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#f0f0f0"
        y: parent.height * .1
    }

    GraphsView {
        id: chartView
        anchors.top: title.bottom
        anchors.left: tableViewItem.right
        anchors.right: background.right
        anchors.bottom: background.bottom

        theme: GraphsTheme {
            id: myTheme
            colorScheme: Qt.Dark
            theme: GraphsTheme.Theme.QtGreen
        }

        PieSeries {
            id: pieSeries
        }

        PieModelMapper {
            id: pieModelMapper
            first: 1
            count: 4
            orientation: Qt.Vertical
            valuesSection: 1
            labelsSection: 0
            series: pieSeries
            model: pieModel
        }
    }
    Column {
        id: legendColumn

        anchors.horizontalCenter: tableViewItem.horizontalCenter
        anchors.verticalCenter: chartView.verticalCenter
        spacing: 2
        Repeater {
            model: pieSeries.legendData.length
            Rectangle {
                id: legend1
                height: 20
                width: 60
                color: pieSeries.legendData[index].color
                Text {
                    id: text1
                    text: pieSeries.legendData[index].label
                }
            }
        }
    }
    Component.onCompleted: () => {
                               updateMapping()
                               showLabels()
                           }
}
