// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtGraphs

Item {
    anchors.fill: parent

    Text {
        id: title
        text: "(Not So) Simple Pie Graph"
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#f0f0f0"
        y: parent.height * .1
    }

    RowLayout {
        id: controls
        anchors.top: title.bottom
        spacing: 12
        uniformCellSizes: true

        CheckBox {
            text: "Show minorities"
            Layout.fillHeight: true

            onCheckedChanged: {
                if (checked) {
                    pieSeries.append("Jaguar", 2.0)
                    pieSeries.append("Ferrari", 1.0)
                    pieSeries.append("Lamborghini", 0.5)
                    pieSeries.append("Bugatti", 0.3)
                    pieSeries.append("McLaren", 0.2)
                    pieSeries.append("Koenigsegg", 0.1)
                    for (let i = 6; i < 12; ++i) {
                        pieSeries.at(i).labelVisible = true
                        console.log("Appended: " + pieSeries.at(i).label)
                    }
                } else {
                    pieSeries.removeMultiple(6, 6)
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Text {
                text: "Angle limit (" + angleslider.value + "°):"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
            }
        }

        Slider {
            id: angleslider
            from: 0
            to: 20
            stepSize: 1
            value: 0
            Layout.fillHeight: true
            onValueChanged: pieSeries.angleSpanVisibleLimit = value
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Text {
                text: "Label visibility mode:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
            }
        }

        ComboBox {
            model: [ "None", "First", "Odd", "Even" ]
            currentIndex: 1
            Layout.fillHeight: true
            onCurrentIndexChanged: {
                switch (currentIndex) {
                case 0: {
                    pieSeries.angleSpanLabelVisibility = PieSeries.LabelVisibility.None
                    break
                }
                case 1: {
                    pieSeries.angleSpanLabelVisibility = PieSeries.LabelVisibility.First
                    break
                }
                case 2: {
                    pieSeries.angleSpanLabelVisibility = PieSeries.LabelVisibility.Odd
                    break
                }
                case 3: {
                    pieSeries.angleSpanLabelVisibility = PieSeries.LabelVisibility.Even
                    break
                }
                }
            }
        }
    }

    GraphsView {
        id: chartView
        width: parent.width
        height: parent.height
        anchors.top: controls.bottom
        anchors.bottom: parent.bottom

        property variant otherSlice: 0

        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.QtGreen
        }

        PieSeries {
            id: pieSeries
            hoverable: true
            PieSlice {
                label: "Volkswagen"
                labelVisible: true
                value: 13.5
            }
            PieSlice {
                label: "Toyota"
                labelVisible: true
                labelPosition: PieSlice.LabelPosition.InsideHorizontal
                labelColor: 'black'
                value: 10.9
            }
            PieSlice {
                label: "Ford"
                labelVisible: true
                labelPosition: PieSlice.LabelPosition.InsideNormal
                labelColor: 'black'
                value: 8.6
            }
            PieSlice {
                label: "Skoda"
                labelVisible: true
                labelPosition: PieSlice.LabelPosition.InsideTangential
                labelColor: 'black'
                value: 8.2
            }
            PieSlice {
                label: "Volvo"
                labelVisible: true
                value: 6.8
            }

            onHoveredChanged: (enabled)=> {
                                  console.log("hoveredChanged:", enabled)
                                  console.log("isHovered:", hovered)
                              }
        }

        Component.onCompleted: {
            otherSlice = pieSeries.append("Others", 52.0);
            otherSlice.labelVisible = true;
            pieSeries.find("Volkswagen").exploded = true;
        }
    }
}
