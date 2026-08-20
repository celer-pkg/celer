// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtGraphs

Item {
    id: root
    anchors.fill: parent

    ColumnLayout {
        id: col
        anchors.fill: root
        spacing: 0

        Rectangle {
            color: "black"
            Layout.preferredHeight: root.height * 0.1
            Layout.fillWidth: true
            border.color: "white"
            border.width: 2
            z: 1

            Text {
                color: "white"
                antialiasing: true
                font.bold: true
                font.family: "Akshar"
                font.pointSize: 16
                anchors.centerIn: parent
                text: "PRIMARY FLIGHT DISPLAY"
            }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: -80
            Layout.rightMargin: -25
            Layout.topMargin: -20
            Layout.bottomMargin: -60

            theme: GraphsTheme {
                backgroundVisible: false
                plotAreaBackgroundColor: "#00000000"
            }

            axisX: ValueAxis {
                max: 10
                subTickCount: 9
                lineVisible: false
                gridVisible: true
                subGridVisible: false
                labelsVisible: false
                visible: false
            }

            axisY: ValueAxis {
                max: 10
                subTickCount: 9
                lineVisible: false
                gridVisible: true
                subGridVisible: false
                labelsVisible: true
                visible: false
            }

            AreaSeries {
                id: lowerArea
                color: "green"

                upperSeries: LineSeries {
                    id: lowerLine
                    XYPoint {x: 0; y: 3}
                    XYPoint {x: 10; y: 4}
                }

                lowerSeries: LineSeries {
                    XYPoint {x: 0; y: 0}
                    XYPoint {x: 10; y: 0}
                }
            }

            //! [1]
            AreaSeries {
                id: upperArea
                color: "cyan"

                upperSeries: LineSeries {
                    XYPoint {x: 0; y: 10}
                    XYPoint {x: 10; y: 10}
                }

                lowerSeries: LineSeries {
                    id: upperLine
                    XYPoint {x: 0; y: 3}
                    XYPoint {x: 10; y: 4}
                }
            }

            FrameAnimation {
                running: true
                onTriggered: {
                    upperLine.replace(0, upperLine.at(0).x, Math.sin(elapsedTime) + 6)
                    upperLine.replace(1, upperLine.at(1).x, Math.cos(elapsedTime) + 6)
                    lowerLine.replace(0, lowerLine.at(0).x, Math.sin(elapsedTime) + 6)
                    lowerLine.replace(1, lowerLine.at(1).x, Math.cos(elapsedTime) + 6)
                    barSet.values = [Math.sin(elapsedTime) + 5]
                }
            }
            //! [1]
        }
    }

    Image {
        anchors.centerIn: parent
        source: "plane-fro.png"
        mipmap: true
        width: 256
        height: 256
        z: 2
    }

    Rectangle {
        anchors.top: col.top
        anchors.left: col.left
        height: 275
        width : 100
        anchors.leftMargin: 30
        anchors.topMargin: 60
        color: "#00000000"

        GraphsView {
            anchors.fill: parent
            anchors.leftMargin: -40
            anchors.bottomMargin: -40

            theme: GraphsTheme {
                backgroundVisible: false
                plotAreaBackgroundColor: "#11000000"
                axisY.labelTextColor: "#000000"
                grid.mainColor: "#000000"
                grid.subColor: "#000000"
                axisY.mainColor: "#000000"
                axisY.subColor: "#000000"
                axisX.mainColor: "#000000"
                axisX.subColor: "#000000"
            }

            axisY: ValueAxis {
                max: 10
                subTickCount: 9
                gridVisible: false
                subGridVisible: false
            }
        }

        GraphsView {
            anchors.fill: parent
            anchors.leftMargin: -40
            anchors.bottomMargin: -40

            theme: GraphsTheme {
                backgroundVisible: false
                plotAreaBackgroundColor: "#00000000"
            }

            axisX: BarCategoryAxis {
                categories: ["PITCH"]
                lineVisible: false
                gridVisible: false
                subGridVisible: false
                labelsVisible: false
                visible: false
            }
            axisY: ValueAxis {
                id: axisY
                max: 10
                subTickCount: 9
                lineVisible: false
                gridVisible: false
                subGridVisible: false
                labelsVisible: false
                visible: false
            }

            //! [2]
            BarSeries {
                id: barSeries
                selectable: true
                barDelegate: Item {
                    id: delegate
                    antialiasing: true
                    property real barOpacity: 0.5
                    property color barColor
                    property string barLabel

                    ShaderEffect {
                        id: effect
                        readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)

                        blending: true
                        fragmentShader: 'pitchbar.frag.qsb'
                        anchors.fill: parent
                    }
                }

                BarSet { id: barSet; values: []; selectedColor: "red" }
            }
            //! [2]
        }

    }

    Rectangle {
        anchors.bottom: col.bottom
        anchors.left: col.left
        anchors.right: col.right
        anchors.leftMargin: 30
        anchors.rightMargin: 30
        anchors.bottomMargin: 0
        height: 100
        clip: true
        color: "transparent"

        //! [3]
        GraphsView {
            anchors.fill: parent
            anchors.leftMargin: -90
            anchors.rightMargin: -80
            anchors.bottomMargin: -30

            theme: GraphsTheme {
                backgroundVisible: false
                plotAreaBackgroundColor: "#11000000"
            }

            axisX: ValueAxis {
                max: 10
                subTickCount: 9
                lineVisible: false
                gridVisible: false
                subGridVisible: false
                labelsVisible: false
                visible: false
            }

            axisY: ValueAxis {
                max: 10
                subTickCount: 9
                lineVisible: false
                gridVisible: false
                subGridVisible: false
                labelsVisible: false
                visible: false
            }

            ToolTip {
                id: tooltip
            }

            onHoverEnter: {
                tooltip.visible = true;
            }

            onHoverExit: {
                tooltip.visible = false;
            }

            onHover: (seriesName, position, value) => {
                         tooltip.x = position.x + 1;
                         tooltip.y = position.y + 1;
                         tooltip.text = "Altitude: " + (value.y * 1000).toFixed(1) + "m";
                     }

            FrameAnimation {
                property var points: []

                Component.onCompleted: {
                    for (let i = 0; i < altitudeLine.count; ++i) {
                        points[i] = altitudeLine.at(i)
                    }
                }

                running: true
                onTriggered: {
                    for (let i = 0; i < points.length; ++i) {
                        points[i].x -= frameTime

                        if (points[1].x <= -2) {
                            let p = points[0]
                            p.x = points[points.length - 1].x + 1
                            altitudeLine.append(p)
                            altitudeLine.remove(0)

                            points.length = 0

                            for (let i = 0; i < altitudeLine.count; ++i) {
                                points[i] = altitudeLine.at(i)
                            }
                        }
                    }
                    altitudeLine.replace(points)
                    altitudeLine.update()
                }
            }

            SplineSeries {
                id: altitudeLine
                hoverable: true
                width: 3

                XYPoint {x: 0; y: 5}
                XYPoint {x: 1; y: 2}
                XYPoint {x: 2; y: 5}
                XYPoint {x: 3; y: 4}
                XYPoint {x: 4; y: 6}
                XYPoint {x: 5; y: 7}
                XYPoint {x: 6; y: 9}
                XYPoint {x: 7; y: 8}
                XYPoint {x: 8; y: 9}
                XYPoint {x: 9; y: 6}
                XYPoint {x: 10; y: 6}
                XYPoint {x: 11; y: 6}
                XYPoint {x: 12; y: 1}
                XYPoint {x: 13; y: 9}
                XYPoint {x: 14; y: 1}
            }

        }
        //! [3]
    }
}
