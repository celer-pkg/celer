// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls.Basic
import "BarsEffect/export"

Rectangle {
    anchors.fill: parent
    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: "#868373"
        }
        GradientStop {
            position: 1.0
            color: "#423f39"
        }
    }

    // Custom component for scalable bar images
    component BarImage: Item {
        property alias topImage: topPart.source
        property alias bottomImage: bottomPart.source
        property alias centerImage: centerPart.source
        Image {
            id: topPart
            width: parent.width
            height: Math.min(width * 0.5, parent.height * 0.5)
            mipmap: true
        }
        Image {
            id: bottomPart
            anchors.bottom: parent.bottom
            width: parent.width
            height: Math.min(width * 0.5, parent.height * 0.5)
            mipmap: true
        }
        Image {
            id: centerPart
            anchors.top: topPart.bottom
            anchors.bottom: bottomPart.top
            width: parent.width
            mipmap: true
            fillMode: Image.TileVertically
        }
    }

    GraphsView {
        id: chartView
        anchors.fill: parent
        anchors.margins: 20 * px
        anchors.topMargin: 20 * px

        axisX: BarCategoryAxis {
            categories: ["2023", "2024", "2025", "2026"]
            gridVisible: false
            subGridVisible: false
        }
        axisY: ValueAxis {
            subTickCount: 9
            tickInterval: 1
            labelDecimals: 1
            SequentialAnimation on max {
                loops: Animation.Infinite
                NumberAnimation {
                    to: 8
                    duration: 3000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 12.5
                    duration: 6000
                    easing.type: Easing.InOutQuad
                }
            }
        }

        theme: GraphsTheme {
            id: myTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.MixSeries
            axisXLabelFont.pixelSize: 20
        }
        BarSeries {
            id: barSeries

            property bool show: true
            property real showAnimated: show

            Behavior on showAnimated {
                NumberAnimation {
                    duration: 1600
                    easing.type: Easing.InOutQuad
                }
            }

            selectable: true
            barWidth: 0.8
            valuesMultiplier: barSeries.showAnimated
            barDelegate: Item {
                id: comp
                property color barColor
                property bool barSelected
                property real barValue
                property string barLabel
                anchors.alignWhenCentered: false
                opacity: barSeries.showAnimated
                BarImage {
                    id: barMask
                    anchors.fill: parent
                    topImage: "images/bar_mask_l.png"
                    bottomImage: "images/bar_mask_r.png"
                    centerImage: "images/bar_mask_c.png"
                    layer.enabled: true
                    visible: false
                }
                BarsEffect {
                    id: barsEffect
                    anchors.fill: parent
                    opacityMaskSource: barMask
                    value: 1.0
                    timeRunning: value > 0
                    color1: Qt.darker(comp.barColor, 1.6)
                    color2: comp.barColor
                    barAngle: barsAngleSlider.sliderValue
                    barSize: 20
                    speed: -40
                }
                BarImage {
                    anchors.fill: parent
                    topImage: "images/bar_fg_l.png"
                    bottomImage: "images/bar_fg_r.png"
                    centerImage: "images/bar_fg_c.png"
                }
                Item {
                    anchors.centerIn: parent
                    anchors.alignWhenCentered: false
                    width: parent.height
                    height: parent.width
                    rotation: -90
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: height * 0.5
                        anchors.right: parent.right
                        anchors.rightMargin: height * 0.5
                        anchors.verticalCenter: parent.verticalCenter
                        text: comp.barLabel
                        elide: Text.ElideRight
                        color: "#ffffff"
                        font.pixelSize: parent.height * 0.4
                        style: Text.Outline
                        styleColor: "#202020"
                    }
                }
                Rectangle {
                    // Show selections
                    anchors.fill: parent
                    color: "transparent"
                    border.width: 3
                    border.color: "#ffffff"
                    radius: width * 0.5
                    opacity: comp.barSelected
                    visible: opacity
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 400
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
                Rectangle {
                    id: valuePopup
                    anchors.top: parent.top
                    anchors.topMargin: parent.width * 0.1 - height * comp.barSelected * 1.2
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width * 0.8
                    height: width
                    radius: width * 0.5
                    border.width: 1
                    color: "#d0202020"
                    border.color: "#606060"
                    scale: Math.max(0, barSeries.showAnimated * 3.0 - 2.0)
                    rotation: 360 - barSeries.showAnimated * 360
                    Text {
                        anchors.centerIn: parent
                        text: comp.barValue.toFixed(1)
                        color: "#d0f0f0f0"
                        font.pixelSize: parent.width * 0.4
                    }
                    Behavior on anchors.topMargin {
                        NumberAnimation {
                            duration: 600
                            easing.type: Easing.OutBack
                        }
                    }
                }
            }
            BarSet { id: set1; label: "HOME"; values: [7.5, 2, 3, 4] }
            BarSet { id: set2; label: "WORK"; values: [8, 2, 6, 6] }
            BarSet { id: set3; label: "HOBBIES"; values: [6, 3, 5 + 2 * Math.sin(fA.elapsedTime), 3] }
            FrameAnimation {
                id: fA
                running: true
            }
        }
    }

    SettingsView {
        Item {
            width: 260
            height: 1
        }
        CustomLabel {
            text: "Effect angle"
        }
        CustomSlider {
            id: barsAngleSlider
            sliderValue: -0.5
            fromValue: -1.0
            toValue: 1.0
        }
        Button {
            width: 250
            text: "Theme 1"
            onClicked: {
                myTheme.theme = GraphsTheme.Theme.MixSeries;
            }
        }
        Button {
            width: 250
            text: "Theme 2"
            onClicked: {
                myTheme.theme = GraphsTheme.Theme.PurpleSeries;
            }
        }
        Button {
            width: 250
            text: "Show / Hide"
            onClicked: {
                barSeries.show = !barSeries.show;
            }
        }
        Button {
            width: 250
            text: "(De)Select All"
            onClicked: {
                if (set1.selectedBars.length == 0)
                    barSeries.selectAll();
                else
                    barSeries.deselectAll();
            }
        }
    }
}
