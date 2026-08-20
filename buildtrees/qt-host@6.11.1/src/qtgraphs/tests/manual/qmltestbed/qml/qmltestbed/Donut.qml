
// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Controls
import QtGraphs

Item {
    anchors.fill: parent

    Text {
        id: title
        text: "Production Costs"
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#f0f0f0"
        y: parent.height * .1
    }

    //![1]
    GraphsView {
        id: chart
        anchors.fill: parent
        antialiasing: true

        PieSeries {
            id: pieOuter
            pieSize: 0.96
            holeSize: 0.7

            PieSlice {
                id: slice
                label: "Alpha"
                value: 19511
                color: "#99CA53"
            }
            PieSlice {
                label: "Epsilon"
                value: 11105
                color: "#209FDF"
            }
            PieSlice {
                label: "Psi"
                value: 9352
                color: "#F6A625"
            }
        }

        PieSeries {
            pieSize: 0.7
            id: pieInner
            holeSize: 0.25

            PieSlice {
                label: "Materials"
                value: 10334
                color: "#B9DB8A"
                labelColor: "#000000"
            }
            PieSlice {
                label: "Employee"
                value: 3066
                color: "#DCEDC4"
                labelColor: "#000000"
            }
            PieSlice {
                label: "Logistics"
                value: 6111
                color: "#F3F9EB"
                labelColor: "#000000"
            }

            PieSlice {
                label: "Materials"
                value: 7371
                color: "#63BCE9"
                labelColor: "#000000"
            }
            PieSlice {
                label: "Employee"
                value: 2443
                color: "#A6D9F2"
                labelColor: "#000000"
            }
            PieSlice {
                label: "Logistics"
                value: 1291
                color: "#E9F5FC"
                labelColor: "#000000"
            }

            PieSlice {
                label: "Materials"
                value: 4022
                color: "#F9C36C"
                labelColor: "#000000"
            }
            PieSlice {
                label: "Employee"
                value: 3998
                color: "#FCE1B6"
                labelColor: "#000000"
            }
            PieSlice {
                label: "Logistics"
                value: 1332
                color: "#FEF5E7"
                labelColor: "#000000"
            }
        }

        Component.onCompleted: {
            // Set the common slice properties dynamically for convenience
            for (var i = 0; i < pieOuter.count; i++) {
                pieOuter.at(i).labelPosition = PieSlice.LabelPosition.Outside
                pieOuter.at(i).labelVisible = true
                pieOuter.at(i).borderWidth = 3
            }
            for (var i = 0; i < pieInner.count; i++) {
                pieInner.at(i).labelPosition = PieSlice.LabelPosition.InsideNormal
                pieInner.at(i).labelVisible = true
                pieInner.at(i).borderWidth = 2
            }
        }
    }

    Row {
        id: sliderRow
        anchors.right: parent.right

        Text {
            id: pieOuterText
            text: "Outer pie valuesMultiplier"
            anchors.verticalCenter: pieOuterSlider.verticalCenter
        }

        Slider {
            id: pieOuterSlider
            from: 0
            value: 1.0
            to: 1.0

            onValueChanged: pieOuter.valuesMultiplier = value
        }

        Text {
            id: pieInnerText
            text: "Inner pie valuesMultiplier"
            anchors.verticalCenter: pieInnerSlider.verticalCenter
        }

        Slider {
            id: pieInnerSlider
            from: 0
            value: 1.0
            to: 1.0

            onValueChanged: pieInner.valuesMultiplier = value
        }
    }
}
//![1]

