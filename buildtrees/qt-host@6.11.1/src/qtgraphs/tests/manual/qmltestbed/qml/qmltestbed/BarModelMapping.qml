// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtGraphs
import QtQuick.Controls.Basic
import QtQuick.Layouts
import TestbedExample

Rectangle {
    id: background
    anchors.fill: parent
    color: "#404040"

    function addMapping() {
        myModel.clearMapping()
        if (vModelMapper.orientation === Qt.Vertical) {
            for (var i = 0; i < barSeries.legendData.length; i++) {
                var cf = vModelMapper.firstBarSetSection + i
                var cl = 1
                var rf = vModelMapper.first
                var rl = barSeries.barSets[i].count
                var c = barSeries.legendData[i].color
                myModel.addMapping(c, cf, rf, cl, rl)
            }
        } else {
            for (var i = 0; i < barSeries.legendData.length; i++) {
                var rf = vModelMapper.firstBarSetSection + i
                var rl = 1
                var cf = vModelMapper.first
                var cl = barSeries.barSets[i].count
                var c = barSeries.legendData[i].color
                myModel.addMapping(c, cf, rf, cl, rl)
            }
        }
    }
    function updateCategoryAxis() {
        if (vModelMapper.orientation === Qt.Vertical) {
            var categories = copyArray(vHeaderView.rowNames)
            var categorySubset = categories.splice(vModelMapper.first,
                                                   vModelMapper.count)
            categoryAxis.categories = categorySubset
        } else {
            var categories = []
            var end = vModelMapper.first + vModelMapper.count
            if (end >= myModel.columnCount())
                end = myModel.columnCount()

            for (var i = vModelMapper.first; i < end; ++i)
                categories.push(myModel.headerData(i, Qt.Horizontal))

            categoryAxis.categories = categories
        }
    }

    function handleOrientationChange() {
        if (vModelMapper.orientation === Qt.Vertical) {
            firstSelectionLabel.text = "first row"
            countSelectionLabel.text = "row count"
            firstSetSectionSelectionLabel.text = "first column"
            lastSetSectionSelectionLabel.text = "last column"
            countSelectionSlider.to = 12
            firstSelectionSlider.to = 12
            firstSetSectionSelectionSlider.to = 5
            lastSetSectionSelectionSlider.to = 5
        } else {
            firstSelectionLabel.text = "first column"
            countSelectionLabel.text = "column count"
            firstSetSectionSelectionLabel.text = "first row"
            lastSetSectionSelectionLabel.text = "last row"
            countSelectionSlider.to = 6
            firstSelectionSlider.to = 5
            firstSetSectionSelectionSlider.to = 12
            lastSetSectionSelectionSlider.to = 12
        }
        myModel.startAddMapping()
        addMapping()
        myModel.endAddMapping()
        updateCategoryAxis()
    }

    function copyArray(arr) {
        var copy = []
        for (var i = 0; i < arr.length; ++i)
            copy.push(arr[i])
        return copy
    }

    Item {
        id: tableViewItem

        anchors.top: background.top
        anchors.left: background.left
        anchors.bottom: background.bottom
        anchors.topMargin: 80 * px
        HorizontalHeaderView {
            id: hHeaderView

            anchors.top: tableViewItem.top
            anchors.left: tableView.left
            syncView: tableView
            Layout.fillWidth: true
            delegate: Text {
                padding: 3
                text: display
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
        VerticalHeaderView {
            id: vHeaderView

            readonly property var rowNames: ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"]
            anchors.top: tableView.top
            anchors.left: tableViewItem.left
            syncView: tableView
            Layout.fillHeight: true
            delegate: Text {
                required property int index
                padding: 3
                text: vHeaderView.rowNames[index]
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
        TableView {
            id: tableView
            anchors.top: hHeaderView.bottom
            anchors.left: vHeaderView.right

            implicitWidth: 300
            implicitHeight: 300
            reuseItems: false
            clip: true
            model: BarModelMapperModel {
                id: myModel
            }
            delegate: Rectangle {
                id: delegateRoot
                required property string display
                required property color background
                implicitHeight: 30
                implicitWidth: tableView.width / 5

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
        Column {
            anchors.top: tableView.bottom
            anchors.left: vHeaderView.left
            anchors.topMargin: 10
            anchors.leftMargin: 10
            Row {
                anchors.horizontalCenter: firstSelectionSlider.horizontalCenter
                Text {
                    id: firstSelectionLabel
                    text: qsTr("first row")
                }
                Text {
                    id: firstSelectionValueLabel
                    leftPadding: 50
                    text: firstSelectionSlider.value
                }
            }

            Slider {
                id: firstSelectionSlider
                from: 0
                to: 11
                stepSize: 1.0
                value: 3
                onMoved: () => {
                             myModel.startAddMapping()
                             vModelMapper.first = firstSelectionSlider.value
                             myModel.endAddMapping()
                             updateCategoryAxis()
                         }
            }
            Row {
                anchors.horizontalCenter: countSelectionSlider.horizontalCenter
                Text {
                    id: countSelectionLabel
                    text: qsTr("row count")
                }
                Text {
                    id: countSelectionValueLabel
                    text: countSelectionSlider.value
                    leftPadding: 50
                }
            }
            Slider {
                id: countSelectionSlider
                from: 1
                to: 12
                value: 5
                stepSize: 1.0
                onMoved: () => {
                             myModel.startAddMapping()
                             vModelMapper.count = countSelectionSlider.value
                             myModel.endAddMapping()
                             updateCategoryAxis()
                         }
            }
            Row {
                anchors.horizontalCenter: firstSetSectionSelectionSlider.horizontalCenter
                Text {
                    id: firstSetSectionSelectionLabel
                    text: qsTr("first column")
                }
                Text {
                    id: firstSetSectionSelectionValueLabel
                    text: firstSetSectionSelectionSlider.value
                    leftPadding: 50
                }
            }
            Slider {
                id: firstSetSectionSelectionSlider
                from: 0
                to: 5
                value: 1
                stepSize: 1.0
                onMoved: () => {
                             myModel.startAddMapping()
                             vModelMapper.firstBarSetSection = firstSetSectionSelectionSlider.value
                             myModel.endAddMapping()
                             updateCategoryAxis()
                         }
            }
            Row {
                anchors.horizontalCenter: lastSetSectionSelectionSlider.horizontalCenter
                Text {
                    id: lastSetSectionSelectionLabel
                    text: qsTr("last column")
                }
                Text {
                    id: lastSetSectionSelectionValueLabel
                    text: lastSetSectionSelectionSlider.value
                    leftPadding: 50
                }
            }
            Slider {
                id: lastSetSectionSelectionSlider
                from: 0
                to: 5
                stepSize: 1.0
                value: 4
                onMoved: () => {
                             myModel.startAddMapping()
                             vModelMapper.lastBarSetSection = lastSetSectionSelectionSlider.value
                             myModel.endAddMapping()
                             updateCategoryAxis()
                         }
            }
        }
    }
    Row {
        id: legendRow
        anchors.top: background.top
        anchors.topMargin: 30
        anchors.horizontalCenter: background.horizontalCenter
        spacing: 2
        Repeater {
            model: barSeries.legendData.length
            Rectangle {
                id: legend1
                height: 20
                width: 40
                color: barSeries.legendData[index].color
                Text {
                    id: text1
                    text: barSeries.legendData[index].label
                }
            }
        }
    }

    Item {
        id: mainView
        anchors.left: tableViewItem.right
        anchors.right: background.right
        anchors.top: background.top
        anchors.bottom: background.bottom

        GraphsView {
            id: chartView
            anchors.fill: mainView
            anchors.margins: 20 * px
            anchors.topMargin: 80 * px
            anchors.leftMargin: 350 * px

            axisX: BarCategoryAxis {
                id: categoryAxis
                categories: ["April", "May", "June", "July", "August"]
                subGridVisible: false
            }
            axisY: ValueAxis {
                id: axisY
                max: 10
                subTickCount: 9
            }

            BarSeries {
                id: barSeries
                selectable: true
                onLegendDataChanged: addMapping()
            }

            BarModelMapper {
                id: vModelMapper
                series: barSeries
                model: myModel
                firstBarSetSection: 1
                lastBarSetSection: 4
                first: 3
                count: 5
                orientation: Qt.Vertical
                onOrientationChanged: handleOrientationChange()
            }
        }
    }
    SettingsView {
        Item {
            width: 260
            height: 10
        }

        Button {
            width: 250
            text: "Vertical/Horizontal bars"
            onClicked: {
                if (chartView.orientation === Qt.Vertical)
                    chartView.orientation = Qt.Horizontal
                else
                    chartView.orientation = Qt.Vertical
                myModel.startAddMapping()
                myModel.endAddMapping()
            }
        }

        Button {
            width: 250
            text: "Vertical/Horizontal mapper"
            onClicked: {
                vModelMapper.orientation = vModelMapper.orientation
                        === Qt.Vertical ? Qt.Horizontal : Qt.Vertical
            }
        }
    }
}
