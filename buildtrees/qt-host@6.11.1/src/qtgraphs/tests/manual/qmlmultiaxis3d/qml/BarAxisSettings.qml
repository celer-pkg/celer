// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtGraphs
import QtQuick
import QtQuick.Layouts
import "."
import QtQuick3D.Helpers
import QtQuick.Controls

Rectangle {
    id: root
    property Abstract3DSeries series
    property int index: 0
    height: seriesTitle.height + seriesAxes.height + 20

    property bool isRowAxis: series.rowAxis != null
    property bool isColumnAxis: series.columnAxis != null
    property bool isValueAxis: series.valueAxis != null

    Text {
        id: seriesTitle
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        horizontalAlignment: Text.AlignHCenter
        text: "Series" + index
    }

    ColumnLayout {
        id: seriesAxes
        anchors.top: seriesTitle.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 2

        CategorySingleAxis {
            id: settingsRow
            graphSeries: series
            axis: rowAxis
            isAxis: isRowAxis
            axisDir: "Row"
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        CategorySingleAxis {
            id: settingsColumn
            graphSeries: series
            axis: columnAxis
            isAxis: isColumnAxis
            axisDir: "Column"
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        SingleAxis {
            id: settingsValue
            graphSeries: series
            axis: valueAxis
            isAxis: isValueAxis
            axisDir: "Value"
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    Category3DAxis {
        id: rowAxis

        labels: settingsRow.labels

        labelsVisible: settingsRow.labelsVisible
        labelAutoAngle: settingsRow.labelAutoAngle
        labelSize: settingsRow.labelSize
        scaleLabelsByCount: settingsRow.scaleLabelsByCount
        min: settingsRow.axisMin
        max: settingsRow.axisMax
        titleVisible: settingsRow.titleVisible
        titleFixed: settingsRow.titleFixed
        titleOffset: settingsRow.titleOffset

        title: qsTr("Series %1 Row").arg(index)
    }

    Category3DAxis {
        id: columnAxis

        labels: settingsColumn.labels

        labelsVisible: settingsColumn.labelsVisible
        labelAutoAngle: settingsColumn.labelAutoAngle
        labelSize: settingsColumn.labelSize
        scaleLabelsByCount: settingsColumn.scaleLabelsByCount
        min: settingsColumn.axisMin
        max: settingsColumn.axisMax
        titleVisible: settingsColumn.titleVisible
        titleFixed: settingsColumn.titleFixed
        titleOffset: settingsColumn.titleOffset

        title: qsTr("Series %1 Column").arg(index)
    }

    Value3DAxis {
        id: valueAxis
        segmentCount: settingsValue.segmentCount
        subSegmentCount: settingsValue.subSegmentCount
        reversed: settingsValue.reversed

        labelsVisible: settingsValue.labelsVisible
        labelAutoAngle: settingsValue.labelAutoAngle
        labelSize: settingsValue.labelSize
        scaleLabelsByCount: settingsValue.scaleLabelsByCount
        min: settingsValue.axisMin
        max: settingsValue.axisMax
        titleVisible: settingsValue.titleVisible
        titleFixed: settingsValue.titleFixed
        titleOffset: settingsValue.titleOffset

        title: qsTr("Series %1 Value").arg(index)
    }
}
