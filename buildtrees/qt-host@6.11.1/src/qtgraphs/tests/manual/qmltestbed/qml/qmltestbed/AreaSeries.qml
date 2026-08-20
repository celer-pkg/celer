// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs

Rectangle {
    id: mainView
    width: 800
    height: 600
    color: "#4F4040"

    SettingsView {
        id: settingsView
        CustomLabel {
            text: "X-coordinate: Min"
        }
        CustomSlider {
            id: sliderMinValueX
            sliderValue: xAxis.min
            fromValue: -40
            toValue: 0
            onSliderValueChanged: xAxis.min = sliderValue;
        }
        CustomLabel {
            text: "X-coordinate: Max"
        }
        CustomSlider {
            id: sliderSpeedValueX
            sliderValue: xAxis.max
            fromValue: 0
            toValue: 40
            onSliderValueChanged: xAxis.max = sliderValue;
        }
        CustomLabel {
            text: "Y-coordinate: Min"
        }
        CustomSlider {
            id: sliderMinValueY
            sliderValue: yAxis.min
            fromValue: -40
            toValue: 0
            onSliderValueChanged: yAxis.min = sliderValue;
        }
        CustomLabel {
            text: "Y-coordinate: Max"
        }
        CustomSlider {
            id: sliderSpeedValueY
            sliderValue: yAxis.max
            fromValue: 0
            toValue: 40
            onSliderValueChanged: yAxis.max = sliderValue;
        }
        CustomCheckBox {
            id: checkBoxDiamond
            text: "flip diamond"
        }
    }

    LineSeries {
        id: low
        XYPoint { x: 3; y: 0.5 }
        XYPoint { x: 4; y: 0.8 }
        XYPoint { x: 5; y: 0.3 }
    }

    LineSeries {
        id: mid
        XYPoint { x: 3; y: 1.7 }
        XYPoint { x: 4; y: 2.3 }
        XYPoint { x: 5; y: 2.4 }
    }

    LineSeries {
        id: high
        XYPoint { x: 3; y: 3.9 }
        XYPoint { x: 4; y: 3.2 }
        XYPoint { x: 5; y: 2.8 }
    }

    LineSeries {
        id: diamondUpperHalf
        XYPoint { x: 4.8; y: 3.4 }
        XYPoint { x: 5.1; y: 3.6 }
        XYPoint { x: 6.1; y: 3.6 }
        XYPoint { x: 6.4; y: 3.4 }
    }

    LineSeries {
        id: diamondLowerHalf
        XYPoint { x: 4.8; y: 3.4 }
        XYPoint { x: 5.6; y: 2.8 }
        XYPoint { x: 6.4; y: 3.4 }
    }

    LineSeries {
        id: changedUpperHalf
        XYPoint { x: 4.8; y: 3.4 }
        XYPoint { x: 5.6; y: 4.0 }
        XYPoint { x: 6.4; y: 3.4 }
    }

    LineSeries {
        id: changedLowerHalf
        XYPoint { x: 4.8; y: 3.4 }
        XYPoint { x: 5.1; y: 3.2 }
        XYPoint { x: 6.1; y: 3.2 }
        XYPoint { x: 6.4; y: 3.4 }
    }

    GraphsView {
        id: chartView
        anchors.fill: parent
        anchors.margins: 20 * px
        anchors.rightMargin: settingsView.posX + 20 * px

        axisX: ValueAxis {
            id: xAxis
            max: 8
        }
        axisY: ValueAxis {
            id: yAxis
            max: 4
        }

        AreaSeries {
            selectable: true

            upperSeries: LineSeries {
                XYPoint { x: 0; y: 1 }
                XYPoint { x: 1; y: 0.5 }
                XYPoint { x: 2; y: 2 }
            }
        }

        AreaSeries {
            color: "#aa4444"
            borderColor: "#882222"
            borderWidth: 3
            selectable: true
            hoverable: true

            upperSeries: LineSeries {
                XYPoint { x: 0; y: 2 }
                XYPoint { x: 1; y: 3.5 }
                XYPoint { x: 2; y: 3.8 }
            }

            lowerSeries: LineSeries {
                XYPoint { x: 0.4; y: 1.5 }
                XYPoint { x: 1; y: 2.5 }
                XYPoint { x: 2.4; y: 3 }
            }
            onHoveredChanged: (enabled)=> {
                                  console.log("hoveredChanged:", enabled)
                                  console.log("isHovered:", hovered)
                              }
        }

        AreaSeries {
            color: "#4444aa"
            borderColor: "#222288"
            borderWidth: 3
            selectable: true

            upperSeries: SplineSeries {
                XYPoint { x: 6; y: 1 }
                XYPoint { x: 7; y: 0.5 }
                XYPoint { x: 8; y: 2 }
            }
        }

        AreaSeries {
            selectable: true

            upperSeries: SplineSeries {
                XYPoint { x: 6; y: 2 }
                XYPoint { x: 7; y: 3.5 }
                XYPoint { x: 8; y: 3.8 }
            }

            lowerSeries: SplineSeries {
                XYPoint { x: 6.4; y: 1.5 }
                XYPoint { x: 7; y: 2.5 }
                XYPoint { x: 8; y: 3 }
            }
        }

        AreaSeries {
            upperSeries: high
            lowerSeries: mid
        }

        AreaSeries {
            upperSeries: mid
            lowerSeries: low
        }

        AreaSeries {
            upperSeries: low
        }

        AreaSeries {
            upperSeries: checkBoxDiamond.checked ? changedUpperHalf : diamondUpperHalf
            lowerSeries: checkBoxDiamond.checked ? changedLowerHalf : diamondLowerHalf
        }

    }
}
