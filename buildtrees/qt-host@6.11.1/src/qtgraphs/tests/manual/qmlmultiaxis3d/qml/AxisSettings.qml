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

    property bool isXAxis: series.axisX != null
    property bool isYAxis: series.axisY != null
    property bool isZAxis: series.axisZ != null

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

        SingleAxis {
            id: settingsX
            graphSeries: series
            axis: axisX
            isAxis: isXAxis
            axisDir: "X"
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        SingleAxis {
            id: settingsY
            graphSeries: series
            axis: axisY
            isAxis: isYAxis
            axisDir: "Y"
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        SingleAxis {
            id: settingsZ
            graphSeries: series
            axis: axisZ
            isAxis: isZAxis
            axisDir: "Z"
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    Value3DAxis {
        id: axisX
        segmentCount: settingsX.segmentCount
        subSegmentCount: settingsX.subSegmentCount
        reversed: settingsX.reversed

        labelsVisible: settingsX.labelsVisible
        labelAutoAngle: settingsX.labelAutoAngle
        labelSize: settingsX.labelSize
        scaleLabelsByCount: settingsX.scaleLabelsByCount
        min: settingsX.axisMin
        max: settingsX.axisMax
        titleVisible: settingsX.titleVisible
        titleFixed: settingsX.titleFixed
        titleOffset: settingsX.titleOffset

        title: qsTr("Series %1 X").arg(index)
    }

    Value3DAxis {
        id: axisY
        segmentCount: settingsY.segmentCount
        subSegmentCount: settingsY.subSegmentCount
        reversed: settingsY.reversed

        labelsVisible: settingsY.labelsVisible
        labelAutoAngle: settingsY.labelAutoAngle
        labelSize: settingsY.labelSize
        scaleLabelsByCount: settingsY.scaleLabelsByCount
        min: settingsY.axisMin
        max: settingsY.axisMax
        titleVisible: settingsY.titleVisible
        titleFixed: settingsY.titleFixed
        titleOffset: settingsY.titleOffset

        title: qsTr("Series %1 Y").arg(index)
    }

    Value3DAxis {
        id: axisZ
        segmentCount: settingsZ.segmentCount
        subSegmentCount: settingsZ.subSegmentCount
        reversed: settingsZ.reversed

        labelsVisible: settingsZ.labelsVisible
        labelAutoAngle: settingsZ.labelAutoAngle
        labelSize: settingsZ.labelSize
        scaleLabelsByCount: settingsZ.scaleLabelsByCount
        min: settingsZ.axisMin
        max: settingsZ.axisMax
        titleVisible: settingsZ.titleVisible
        titleFixed: settingsZ.titleFixed
        titleOffset: settingsZ.titleOffset

        title: qsTr("Series %1 Z").arg(index)
    }
}
