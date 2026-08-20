// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs

Rectangle {
    anchors.fill: parent
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#d0d020" }
        GradientStop { position: 1.0; color: "#406020" }
    }

    GraphsView {
        id: chartView
        anchors.fill: parent
        anchors.margins: 50 * px
        anchors.rightMargin: settingsView.posX + 60 * px
        shadowVisible: checkBoxShadowVisible.checked
        shadowColor: "#80404040"
        axisX: BarCategoryAxis {
            id: xAxis
            visible: checkBoxAxisXVisible.checked
            lineVisible: checkBoxAxisXLineVisible.checked
            labelsVisible: checkBoxAxisXLabelsVisible.checked
            categories: [1, 2, 3, 4, 5, 6]
            gridVisible: checkBoxGridXMajor.checked
            subGridVisible: checkBoxGridXMinor.checked
            alignment: checkBoxAxisXAlignment.checked ? Qt.AlignTop : Qt.AlignBottom
            labelPosition: axisXLabelPosition.checked ? BarCategoryAxis.LabelPosition.Center
                                                       : BarCategoryAxis.LabelPosition.OnValue
        }
        axisY: ValueAxis {
            id: yAxis
            visible: checkBoxAxisYVisible.checked
            lineVisible: checkBoxAxisYLineVisible.checked
            labelsVisible: checkBoxAxisYLabelsVisible.checked
            max: 10
            subTickCount: 1
            // Alternative tick formatting
            //tickInterval: 1.0
            //labelFormat: "g"
            //labelDecimals: 3
            gridVisible: checkBoxGridYMajor.checked
            subGridVisible: checkBoxGridYMinor.checked
            alignment: checkBoxAxisYAlignment.checked ? Qt.AlignRight : Qt.AlignLeft
        }
        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            backgroundColor: "#80000000"
            plotAreaBackgroundColor: "#20000000"
            seriesColors: ["#dd444444", "#dd555555", "#dd666666", "#dd777777", "#dd888888"]
            borderColors: ["#111111", "#222222", "#333333", "#444444", "#555555"]
            borderWidth: 2
            axisXLabelFont.pixelSize: 20
            grid.mainColor: "#ffffff"
            grid.subColor: "#eeeeee"
            axisY.mainColor: "#ffffff"
            axisY.subColor: "#eeeeee"
            axisX.mainColor: "#ffffff"
            axisX.subColor: "#eeeeee"
        }
        BarSeries {
            BarSet { id: set1; label: "Bob"; values: [1, 2, 3, 4, 5, 6] }
            BarSet { id: set2; label: "Frank"; values: [8, 8, 6, 0, 5, 3] }
            BarSet { id: set3; label: "James"; values: [4, 3, 2, 6, 4, 2] }
            BarSet { id: set5; label: "Frank"; values: [8, 4, 3, 1, 8, 5] }
            BarSet { id: set6; label: "James"; values: [5, 2, 5, 7, 1, 2] }
            barDelegate: checkBoxCustomBars.checked ? customComponent : null
            Component {
                id: customComponent
                Rectangle {
                    id: comp
                    property color barColor
                    property int barIndex
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: (comp.barIndex % 2 === 0) ? "#f02020" : "#d0d020"}
                        GradientStop { position: 0.4; color: Qt.darker(comp.barColor, 2.0) }
                        GradientStop { position: 1.0; color: comp.barColor }
                    }
                    radius: width / 2
                    border.width: 1
                    border.color: "#808080"
                }
            }
        }
    }
    Rectangle {
        x: chartView.x + chartView.plotArea.x
        y: chartView.y + chartView.plotArea.y
        width: chartView.plotArea.width
        height: chartView.plotArea.height
        color: "transparent"
        border.color: "red"
        border.width: 2
        visible: checkBoxShowPlotArea.checked
    }

    SettingsView {
        id: settingsView
        CustomLabel {
            text: "Margin: Left"
        }
        CustomSlider {
            sliderValue: chartView.marginLeft
            fromValue: 0
            toValue: 60
            onSliderValueChanged: chartView.marginLeft = sliderValue;
        }
        CustomLabel {
            text: "Margin: Right"
        }
        CustomSlider {
            sliderValue: chartView.marginRight
            fromValue: 0
            toValue: 60
            onSliderValueChanged: chartView.marginRight = sliderValue;
        }
        CustomLabel {
            text: "Margin: Top"
        }
        CustomSlider {
            sliderValue: chartView.marginTop
            fromValue: 0
            toValue: 60
            onSliderValueChanged: chartView.marginTop = sliderValue;
        }
        CustomLabel {
            text: "Margin: Bottom"
        }
        CustomSlider {
            sliderValue: chartView.marginBottom
            fromValue: 0
            toValue: 60
            onSliderValueChanged: chartView.marginBottom = sliderValue;
        }
        CustomLabel {
            text: "Y-coordinate: Max"
        }
        CustomSlider {
            id: sliderYMaxValue
            sliderValue: yAxis.max
            fromValue: 0.1
            toValue: 110
            onSliderValueChanged: yAxis.max = sliderValue;
        }
        CustomLabel {
            text: "Y-coordinate: Min"
        }
        CustomSlider {
            id: sliderYMinValue
            sliderValue: yAxis.min
            fromValue: -3
            toValue: 3
            onSliderValueChanged: yAxis.min = sliderValue;
        }
        CustomLabel {
            text: "Y-coordinate: Anchor"
        }
        CustomSlider {
            id: sliderYAnchorValue
            sliderValue: yAxis.tickAnchor
            fromValue: -10
            toValue: 10
            onSliderValueChanged: yAxis.tickAnchor = sliderValue;
        }
        CustomLabel {
            text: "Y-coordinate: Decimals"
        }
        CustomSlider {
            id: sliderYAnchorDecimals
            sliderValue: yAxis.labelDecimals
            sliderStepSize: 1
            fromValue: -1
            toValue: 4
            onSliderValueChanged: yAxis.labelDecimals = sliderValue;
        }
        CustomCheckBox {
            id: checkBoxCustomBars
            text: "Use Custom Bars"
            checked: false
        }
        CustomCheckBox {
            id: checkBoxShowPlotArea
            text: "Show Plot Area"
            checked: false
        }
        CustomCheckBox {
            id: checkBoxAxisXAlignment
            text: "Axis X: Bottom/Top"
            checked: false
        }
        CustomCheckBox {
            id: checkBoxAxisYAlignment
            text: "Axis Y: Left/Right"
            checked: false
        }
        CustomCheckBox {
            id: checkBoxAxisXVisible
            text: "Axis X: Visible"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxAxisXLineVisible
            text: "Axis X: Line visible"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxAxisXLabelsVisible
            text: "Axis X: Labels visible"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxAxisYVisible
            text: "Axis Y: Visible"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxAxisYLineVisible
            text: "Axis Y: Line visible"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxAxisYLabelsVisible
            text: "Axis Y: Labels visible"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxGridYMajor
            text: "Grid Y: Major lines"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxGridYMinor
            text: "Grid Y: Sublines"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxGridXMajor
            text: "Grid X: Major lines"
            checked: false
        }
        CustomCheckBox {
            id: checkBoxGridXMinor
            text: "Grid X: Subticks"
            checked: false
        }

        CustomLabel {
            text: "Y-coordinate: Subtick count"
        }
        CustomSlider {
            id: sliderYTicksValue
            sliderValue: yAxis.subTickCount
            sliderStepSize: 1
            fromValue: 0
            toValue: 9
            onSliderValueChanged: yAxis.subTickCount = sliderValue;
        }
        CustomLabel {
            text: "Grid lines width"
        }
        CustomSlider {
            sliderValue: myTheme.grid.mainWidth
            fromValue: 1.0
            toValue: 4.0
            onSliderValueChanged: {
                myTheme.grid.mainWidth = sliderValue;
                myTheme.axisY.mainWidth = sliderValue;
                myTheme.axisX.mainWidth = sliderValue;
            }
        }
        CustomLabel {
            text: "Subgrid lines width"
        }
        CustomSlider {
            sliderValue: myTheme.grid.subWidth
            fromValue: 1.0
            toValue: 4.0
            onSliderValueChanged: {
                myTheme.grid.subWidth = sliderValue;
                myTheme.axisY.subWidth = sliderValue;
                myTheme.axisX.subWidth = sliderValue;
            }
        }
        CustomCheckBox {
            id: checkBoxShadowVisible
            text: "Shadow: Visible"
            checked: true
        }
        CustomLabel {
            text: "Shadow: Opacity"
        }
        CustomSlider {
            id: sliderShadowOpacity
            sliderValue: chartView.shadowColor.a
            fromValue: 0
            toValue: 1
            onSliderValueChanged: chartView.shadowColor.a = sliderValue;
        }
        CustomLabel {
            text: "Shadow: Width"
        }
        CustomSlider {
            id: sliderShadowWidth
            sliderValue: chartView.shadowBarWidth
            fromValue: 1.0
            toValue: 4.0
            onSliderValueChanged: chartView.shadowBarWidth = sliderValue;
        }
        CustomLabel {
            text: "Shadow: Smoothing"
        }
        CustomSlider {
            id: sliderShadowSmoothing
            sliderValue: chartView.shadowSmoothing
            fromValue: 0
            toValue: 10
            onSliderValueChanged: chartView.shadowSmoothing = sliderValue;
        }
        CustomLabel {
            text: "Shadow: OffsetX"
        }
        CustomSlider {
            id: sliderShadowOffsetX
            sliderValue: chartView.shadowXOffset
            fromValue: -2
            toValue: 2
            onSliderValueChanged: chartView.shadowXOffset = sliderValue;
        }
        CustomLabel {
            text: "Shadow: OffsetY"
        }
        CustomSlider {
            id: sliderShadowOffsetY
            sliderValue: chartView.shadowYOffset
            fromValue: -2
            toValue: 2
            onSliderValueChanged: chartView.shadowYOffset = sliderValue;
        }
        CustomLabel {
            text: "Axis X Label Position (uncheck for OnValue)"
        }
        CustomCheckBox {
            id: axisXLabelPosition
            text: "PositionCenter"
            checked: true
        }
    }
}
