// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick.Controls
import QtQuick.Layouts
import QtQuick
import QtGraphs

Item {
    id: root
    required property Category3DAxis axis
    property Abstract3DSeries graphSeries
    required property string axisDir
    required property bool isAxis

    property list<string> labels: [0]
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
            if (axisDir === "Row") {
                var seriesAxis = graphSeries.rowAxis;
                if (seriesAxis == null) {
                    graphSeries.rowAxis = axis;
                } else {
                    graphSeries.rowAxis = null;
                }
            }
            if (axisDir === "Column") {
                var seriesAxis = graphSeries.columnAxis;
                if (seriesAxis == null) {
                    graphSeries.columnAxis = axis;
                } else {
                    graphSeries.columnAxis = null;
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
                    text: "Labels Count: "
                }
                Slider {
                    id: labelsSlider
                    from: 1
                    to: 10
                    value: 1
                    stepSize: 1
                    onValueChanged: {
                        root.labels = [];
                        for (let i = 0; i < value; i++) {
                            root.labels.push(i);
                        }
                    }
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
