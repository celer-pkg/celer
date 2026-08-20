// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick.Controls
import QtQuick.Layouts
import QtQuick
import QtGraphs

Item {
    required property Value3DAxis axis
    property Abstract3DSeries graphSeries
    required property string axisDir
    required property bool isAxis

    property real segmentCount: segmentsSlider.value
    property real subSegmentCount: subSegmentsSlider.value
    property bool reversed: reversedCB.checked

    property bool labelsVisible: labelsVisibleCB.checked
    property real labelAutoAngle: autoAngleSlider.value
    property real labelSize: sizeSlider.value
    property bool scaleLabelsByCount: scaleLabelsByCountCB.checked
    property real axisMin: minSlider.value
    property real axisMax: maxSlider.value
    property bool titleVisible: titleVisibleCB.checked
    property bool titleFixed: titleFixedCB.checked
    property real titleOffset: titleOffsetSlider.value

    height: 200

    Button {
        id: axisButton
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: isAxis ? qsTr("Remove %1 Axis").arg(axisDir) : qsTr("Add %1 Axis").arg(axisDir)
        onClicked: {
            if (axisDir === "X") {
                var seriesAxis = graphSeries.axisX;
                if (seriesAxis == null) {
                    graphSeries.axisX = axis;
                } else {
                    graphSeries.axisX = null;
                }
            }
            if (axisDir === "Y") {
                var seriesAxis = graphSeries.axisY;
                if (seriesAxis == null) {
                    graphSeries.axisY = axis;
                } else {
                    graphSeries.axisY = null;
                }
            }
            if (axisDir === "Z") {
                var seriesAxis = graphSeries.axisZ;
                if (seriesAxis == null) {
                    graphSeries.axisZ = axis;
                } else {
                    graphSeries.axisZ = null;
                }
            }
            if (axisDir === "Value") {
                var seriesAxis = graphSeries.axisZ;
                if (seriesAxis == null) {
                    graphSeries.valueAxis = axis;
                } else {
                    graphSeries.valueAxis = null;
                }
            }
        }
    }

    ListView {
        id: buttonList
        height: isAxis ? 180 : 0
        visible: isAxis
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: axisButton.bottom
        clip: true

        model: ObjectModel {

            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Segments: "
                }
                Slider {
                    id: segmentsSlider
                    Layout.alignment: Qt.AlignHCenter
                    from: 1
                    to: 10
                    value: 5
                    stepSize: 1
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("%1 ").arg(segmentsSlider.value)
                }
            }

            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Subsegments: "
                }
                Slider {
                    id: subSegmentsSlider
                    Layout.alignment: Qt.AlignHCenter
                    from: 1
                    to: 10
                    value: 1
                    stepSize: 1
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("%1 ").arg(subSegmentsSlider.value)
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Reversed : "
                }
                CheckBox {
                    id: reversedCB
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Labels Visible : "
                }
                CheckBox {
                    id: labelsVisibleCB
                    checked: true
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Labels Auto Angle : "
                }
                Slider {
                    id: autoAngleSlider
                    from: 0
                    to: 90
                    value: 90
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Label Size: "
                }
                Slider {
                    id: sizeSlider
                    from: 1
                    to: 10
                    value: 1
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Scale labels by count: "
                }
                CheckBox {
                    id: scaleLabelsByCountCB
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Axis min: "
                }
                Slider {
                    id: minSlider
                    from: -5
                    to: 5
                    value: 0
                    stepSize: 0.5
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("%1 ").arg(minSlider.value)
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Axis max: "
                }
                Slider {
                    id: maxSlider
                    from: 1
                    to: 20
                    value: 10
                    stepSize: 0.5
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("%1 ").arg(maxSlider.value)
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Title visible: "
                }
                CheckBox {
                    id: titleVisibleCB
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Title fixed: "
                }
                CheckBox {
                    id: titleFixedCB
                }
            }
            RowLayout {
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Title offset: "
                }
                Slider {
                    id: titleOffsetSlider
                    from: -1
                    to: 1
                    value: 0
                }
            }
        }
    }
}
