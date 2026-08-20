// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs
import QtQuick.Window
import Qt.labs.qmlmodels
import "."

Item {
    id: mainview

    property int buttonLayoutHeight: 180;
    property int currentRow
    property Bar3DSeries selectedSeries

    function toggleRowColorsForBarSeries(enable) {
        if (enable)
            barSeries.rowColors = [color1, color2, color3]
        else
            barSeries.rowColors = []
    }

    function toggleRowColorsForSecondarySeries(enable) {
        if (enable)
            secondarySeries.rowColors = [color4, color5, color6]
        else
            secondarySeries.rowColors = []
    }

    function handleSelectionChange(series, position) {
        if (position !== series.invalidSelectionPosition)
            selectedSeries = series

        // Set tableView current row to selected bar
        var rowRole = series.rowLabels[position.x];
        var colRole
        if (barGraph.columnAxis === graphAxes.total)
            colRole = "01";
        else
            colRole = series.columnLabels[position.y];
        var checkTimestamp = rowRole + "-" + colRole

        if (currentRow === -1 || checkTimestamp !== graphData.model.get(currentRow).timestamp) {
            var totalRows = tableView.rows;
            for (var i = 0; i < totalRows; i++) {
                var modelTimestamp = graphData.model.get(i).timestamp
                if (modelTimestamp === checkTimestamp) {
                    currentRow = i
                    break
                }
            }
        }
    }

    width: 1280
    height: 1024

    state: Screen.width < Screen.height ? "portrait" : "landscape"
    selectedSeries: barSeries

    onCurrentRowChanged: {
        var timestamp = graphData.model.get(currentRow).timestamp
        var pattern = /(\d\d\d\d)-(\d\d)/
        var matches = pattern.exec(timestamp)
        var rowIndex = modelProxy.rowCategoryIndex(matches[1])
        var colIndex
        if (barGraph.columnAxis === graphAxes.total)
            colIndex = 0 // Just one column when showing yearly totals
        else
            colIndex = modelProxy.columnCategoryIndex(matches[2])
        if (selectedSeries.visible)
            mainview.selectedSeries.selectedBar = Qt.point(rowIndex, colIndex)
        else if (barSeries.visible)
            barSeries.selectedBar = Qt.point(rowIndex, colIndex)
        else
            secondarySeries.selectedBar = Qt.point(rowIndex, colIndex)
    }

    Data {
        id: graphData
    }

    Axes {
        id: graphAxes
    }

    Color {
        id: color1
        color: "green"
    }

    Color {
        id: color2
        color: "blue"
    }

    Color {
        id: color3
        color: "red"
    }

    Color {
        id: color4
        color: "yellow"
    }

    Color {
        id: color5
        color: "purple"
    }

    Color {
        id: color6
        color: "orange"
    }

    GraphsTheme {
        id: theme1
        theme: GraphsTheme.Theme.YellowSeries
        labelBorderVisible: true
        labelFont.pointSize: 35
        labelBackgroundVisible: true
        colorStyle: GraphsTheme.ColorStyle.Uniform
    }

    GraphsTheme {
        id: theme2
        theme: GraphsTheme.Theme.QtGreenNeon
        labelBorderVisible: true
        labelFont.pointSize: 35
        labelBackgroundVisible: true
        colorStyle: GraphsTheme.ColorStyle.Uniform
    }

    ColumnLayout {
        id: tableViewLayout

        anchors.top: parent.top
        anchors.left: parent.left

        HorizontalHeaderView {
            id: header
            property var columnNames: ["Month", "Expenses", "Income"]

            syncView: tableView
            Layout.fillWidth: true
            delegate: Text {
                text: header.columnNames[index]
            }

        }

        TableView {
            id: tableView
            Layout.fillWidth: true
            Layout.fillHeight: true

            reuseItems: false
            clip: true

            model: TableModel {
                id: tableModel
                TableModelColumn { display: "timestamp" }
                TableModelColumn { display: "expenses" }
                TableModelColumn { display: "income" }

                rows: graphData.modelAsJsArray
            }

            delegate: Rectangle {
                implicitHeight: 30
                implicitWidth: tableView.width / 3
                color: row === currentRow ? "#e0e0e0" : "#ffffff"
                MouseArea {
                    anchors.fill: parent
                    onClicked: currentRow = row
                }

                Text {
                    id: delegateText
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    anchors.leftMargin: 4
                    anchors.left: parent.left
                    anchors.right: parent.right
                    text: formattedText
                    property string formattedText: {
                        if (column === 0) {
                            if (display !== "") {
                                var pattern = /(\d\d\d\d)-(\d\d)/
                                var matches = pattern.exec(display)
                                var colIndex = parseInt(matches[2], 10) - 1
                                return matches[1] + " - " + graphAxes.column.labels[colIndex]
                            }
                        } else {
                            return display
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        id: controlLayout
        spacing: 0

        Button {
            id: seriesToggle
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: "Show Expenses"
            clip: true

            onClicked: {
                if (text === "Show Expenses") {
                    barSeries.visible = false
                    secondarySeries.visible = true
                    barGraph.valueAxis.labelFormat = "-%.2f M\u20AC"
                    secondarySeries.itemLabelFormat = "Expenses, @colLabel, @rowLabel: @valueLabel"
                    text = "Show Both"
                } else if (text === "Show Both") {
                    barSeries.visible = true
                    barGraph.valueAxis.labelFormat = "%.2f M\u20AC"
                    secondarySeries.itemLabelFormat = "Expenses, @colLabel, @rowLabel: -@valueLabel"
                    text = "Show Income"
                } else { // text === "Show Income"
                    secondarySeries.visible = false
                    text = "Show Expenses"
                }
            }
        }

        Button {
            id: themeToggle
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: "Use theme 2"
            clip: true

            onClicked: {
                if (text === "Use theme 2") {
                    barGraph.theme = theme2
                    text = "Use theme 1"
                } else {
                    barGraph.theme = theme1
                    text = "Use theme 2"
                }
            }
        }

        Slider {
            id: shadowSlider
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            from: 0
            to: 100
            value: barGraph.shadowStrength

            onValueChanged: barGraph.shadowStrength = value
        }

        Button {
            id: barSeriesRowColorToggle
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: "Disable row colors"

            onClicked: {
                if (text === "Disable row colors") {
                    toggleRowColorsForBarSeries(false)
                    toggleRowColorsForSecondarySeries(false)
                    text = "Enable row colors"
                } else {
                    toggleRowColorsForBarSeries(true)
                    toggleRowColorsForSecondarySeries(true)
                    text = "Disable row colors"
                }
            }
        }

        Button {
            id: labelsToggle
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: "Hide labels"
            clip: true

            onClicked: {
                if (text === "Hide labels") {
                    barGraph.theme.labelsVisible = false;
                    text = "Show labels";
                } else {
                    barGraph.theme.labelsVisible = true;
                    text = "Hide labels";
                }
            }
        }

        Button {
            id: itemlabelVisibleToggle
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: "Hide itemlabel"
            clip: true

            onClicked: {
                barSeries.itemLabelVisible = !barSeries.itemLabelVisible

                if (barSeries.itemLabelVisible)
                    text = "Hide itemLabel"
                else
                    text = "Show itemLabel"
            }
        }

        Column {
            Label {
                text: "ValueAxis Segments"
            }
        }

        Slider {
            id: valueAxisSegmentSliders
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            from: graphAxes.minAxisSegmentCount
            to: graphAxes.maxAxisSegmentCount
            value: 5

            onValueChanged: barGraph.valueAxis.segmentCount = value
        }

        Slider {
            id: valueAxisSubSegmentSliders
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            from: graphAxes.minAxisSegmentCount
            to: graphAxes.maxAxisSegmentCount
            value: 5

            onValueChanged: barGraph.valueAxis.subSegmentCount = value
        }

        Column {
            Label {
                text: "Rotate bars"
            }
        }

        Slider {
            id: barsRotationSlider
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            from: 0
            to: 360
            value: 5

            onValueChanged: barSeries.meshAngle = value
        }
    }

    Item {
        id: dataView
        anchors.right: mainview.right;
        anchors.bottom: mainview.bottom

        Bars3D {
            id: barGraph
            width: dataView.width
            height: dataView.height
            shadowQuality: Graphs3D.ShadowQuality.Medium
            selectionMode: Graphs3D.SelectionFlag.ItemAndRow | Graphs3D.SelectionFlag.Slice
            theme: theme1
            barThickness: 0.7
            barSpacing: Qt.size(0.5, 0.5)
            barSpacingRelative: false
            cameraPreset: Graphs3D.CameraPreset.IsometricLeftHigh
            columnAxis: graphAxes.column
            rowAxis: graphAxes.row
            valueAxis: graphAxes.value

            Bar3DSeries {
                id: secondarySeries
                visible: false
                itemLabelFormat: "Expenses, @colLabel, @rowLabel: -@valueLabel"
                rowColors: [color4 , color5, color6]

                onSelectedBarChanged: (position)=> handleSelectionChange(secondarySeries, position)

                ItemModelBarDataProxy {
                    id: secondaryProxy
                    itemModel: graphData.model
                    rowRole: "timestamp"
                    columnRole: "timestamp"
                    valueRole: "expenses"
                    rowRolePattern: /^(\d\d\d\d).*$/
                    columnRolePattern: /^.*-(\d\d)$/
                    valueRolePattern: /-/
                    rowRoleReplace: "\\1"
                    columnRoleReplace: "\\1"
                    multiMatchBehavior: ItemModelBarDataProxy.MultiMatchBehavior.Cumulative
                }
            }

            Bar3DSeries {
                id: barSeries
                itemLabelFormat: "Income, @colLabel, @rowLabel: @valueLabel"
                rowColors: [color1, color2, color3]

                onSelectedBarChanged: (position)=> handleSelectionChange(barSeries, position)

                ItemModelBarDataProxy {
                    id: modelProxy
                    itemModel: graphData.model
                    rowRole: "timestamp"
                    columnRole: "timestamp"
                    valueRole: "income"
                    rowRolePattern: /^(\d\d\d\d).*$/
                    columnRolePattern: /^.*-(\d\d)$/
                    rowRoleReplace: "\\1"
                    columnRoleReplace: "\\1"
                    multiMatchBehavior: ItemModelBarDataProxy.MultiMatchBehavior.Cumulative
                }
            }
        }
    }

    states: [
        State  {
            name: "landscape"
            PropertyChanges {
                target: dataView
                width: mainview.width / 4 * 3
                height: mainview.height
            }
            PropertyChanges  {
                target: tableViewLayout
                height: mainview.height - buttonLayoutHeight
                anchors.right: dataView.left
                anchors.left: mainview.left
                anchors.bottom: undefined
            }
            PropertyChanges  {
                target: controlLayout
                width: mainview.width / 4
                height: buttonLayoutHeight
                anchors.top: tableViewLayout.bottom
                anchors.bottom: mainview.bottom
                anchors.left: mainview.left
                anchors.right: dataView.left
            }
        },
        State  {
            name: "portrait"
            PropertyChanges {
                target: dataView
                width: mainview.height / 4 * 3
                height: mainview.width
            }
            PropertyChanges  {
                target: tableViewLayout
                height: mainview.width
                anchors.right: controlLayout.left
                anchors.left: mainview.left
                anchors.bottom: dataView.top
            }
            PropertyChanges  {
                target: controlLayout
                width: mainview.height / 4
                height: mainview.width / 4
                anchors.top: mainview.top
                anchors.bottom: dataView.top
                anchors.left: undefined
                anchors.right: mainview.right
            }
        }
    ]
}
