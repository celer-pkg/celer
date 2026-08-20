// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Fusion
import QtQuick.Dialogs
import QtGraphs

ColumnLayout {
    spacing: 10

    property alias testgridChange: testgridChange

    Label {
        visible: testgridChange.checked
        text: "Grid Width"
        color: "gray"
    }
    Slider {
        visible: testgridChange.checked
        from: 0.0
        to: 10.0
        value: customTheme.grid.mainWidth
        onValueChanged: {
            if (testgridChange.checked) {
                customTheme.grid.mainWidth = value
            }
        }
    }

    Row {
        Label {
            text: "Label text color"
            color: "gray"
        }

        Button {
            height: 25
            width: 25

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5
                color: customTheme.labelTextColor
            }

            onClicked: textCol.open()
        }
    }
    ColorDialog {
        id: textCol
        selectedColor: customTheme.labelTextColor
        onAccepted: customTheme.labelTextColor = selectedColor
    }

        Row {
        Label {
            text: "Axis label text color"
            color: "gray"
        }
        Button {
            height: 25
            width: 25

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5
                color: customTheme.axisX.labelTextColor
            }

            onClicked: axisLabelTextCol.open()
        }
    }
    ColorDialog {
        id: axisLabelTextCol
        selectedColor: customTheme.axisX.labelTextColor
        onAccepted: {
            customTheme.axisX.labelTextColor = selectedColor
            customTheme.axisY.labelTextColor = selectedColor
            customTheme.axisZ.labelTextColor = selectedColor
        }
    }

    Row {
        Label {
            text: "Background color"
            color: "gray"
        }

        Button {
            height: 25
            width: 25

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5
                color: customTheme.backgroundColor
            }

            onClicked: backgroundCol.open()
        }
    }
    ColorDialog {
        id: backgroundCol
        selectedColor: customTheme.backgroundColor
        onAccepted: customTheme.backgroundColor = selectedColor
    }

    Row {
        Label {
            text: "Plot area color"
            color: "gray"
        }

        Button {
            height: 25
            width: 25

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5
                color: customTheme.plotAreaBackgroundColor
            }

            onClicked: plotareaCol.open()
        }
    }
    ColorDialog {
        id: plotareaCol
        selectedColor: customTheme.plotAreaBackgroundColor
        onAccepted: customTheme.plotAreaBackgroundColor = selectedColor
    }

    Row {
        Label {
            text: "Grid color"
            color: "gray"
        }

        Button {
            height: 25
            width: 25

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5
                color: customTheme.grid.mainColor
            }

            onClicked: gridlineCol.open()
        }
    }
    ColorDialog {
        id: gridlineCol
        selectedColor: customTheme.grid.mainColor
        onAccepted: customTheme.grid.mainColor = selectedColor
    }

    Row {
        Label {
            text: "Subgrid color"
            color: "gray"
        }

        Button {
            height: 25
            width: 25

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5
                color: customTheme.grid.subColor
            }

            onClicked: subgridlineCol.open()
        }
    }
    ColorDialog {
        id: subgridlineCol
        selectedColor: customTheme.grid.subColor
        onAccepted: customTheme.grid.subColor = selectedColor
    }

    Label {
        text: "Subsegments"
        color: "gray"
    }
    Slider {
        from: 1
        to: 5
        stepSize: 1
        snapMode: Slider.SnapAlways
        value: bars.visible? bars.valueAxis.subSegmentCount : surface.axisX.subSegmentCount
        onValueChanged: {
            surface.axisX.subSegmentCount = value
            surface.axisY.subSegmentCount = value
            surface.axisZ.subSegmentCount = value
            bars.valueAxis.subSegmentCount = value
        }
    }

    Row {
        Label {
            text: "Background"
            color: "gray"
        }
        CheckBox {
            checked: customTheme.backgroundVisible
            onCheckedChanged: {
                customTheme.backgroundVisible = checked
            }
        }
    }

    Row {
        Label {
            text: "Plot area"
            color: "gray"
        }
        CheckBox {
            checked: customTheme.plotAreaBackgroundVisible
            onCheckedChanged: {
                customTheme.plotAreaBackgroundVisible = checked
            }
        }
    }

    Row {
        Label {
            text: "Grid"
            color: "gray"
        }
        CheckBox {
            checked: customTheme.gridVisible
            onCheckedChanged: {
                customTheme.gridVisible = checked
            }
        }
    }

    Row {
        Label {
            text: "Geometry/Shader Grid"
            color: "gray"
        }
        CheckBox {
            id: testgridChange
            checked: false
            onCheckedChanged: {
                if (checked) {
                    surface.gridLineType = Graphs3D.GridLineType.Shader;
                    bars.gridLineType = Graphs3D.GridLineType.Shader;
                } else {
                    surface.gridLineType = Graphs3D.GridLineType.Geometry;
                    bars.gridLineType = Graphs3D.GridLineType.Geometry;
                }
            }
        }
    }

    Row {
        Label {
            text: "Labels"
            color: "gray"
        }
        CheckBox {
            checked: customTheme.labelsVisible
            onCheckedChanged: {
                customTheme.labelsVisible = checked
            }
        }
    }

    Row {
        Label {
            text: "Label Background"
            color: "gray"
        }
        CheckBox {
            checked: customTheme.labelBackgroundVisible
            onCheckedChanged: {
                customTheme.labelBackgroundVisible = checked
            }
        }
    }

    Row {
        Label {
            text: "Label Border"
            color: "gray"
        }
        CheckBox {
            checked: customTheme.labelBorderVisible
            onCheckedChanged: {
                customTheme.labelBorderVisible = checked
            }
        }
    }
    Label {
        text: "Label Margin"
        color: "gray"
    }
    Slider {
        from: -0.3
        to: 0.3
        value: surface.labelMargin
        onValueChanged: {
                surface.labelMargin = value
        }
    }
}
