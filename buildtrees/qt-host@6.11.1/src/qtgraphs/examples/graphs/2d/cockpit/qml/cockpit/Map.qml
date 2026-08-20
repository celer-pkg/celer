// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes
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
                text: "NAVIGATION MAP"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                id: navColumn
                color: "black"
                Layout.preferredWidth: root.width * 0.2
                Layout.preferredHeight: root.height
                border.color: "white"
                border.width: 2

                GraphsView {
                    anchors.fill: parent
                    orientation: Qt.Horizontal
                    anchors.leftMargin: -60
                    anchors.rightMargin: -60

                    Component {
                        id: triangle

                        Shape {
                            anchors.centerIn: parent
                            ShapePath {
                                strokeWidth: 0
                                strokeColor: "red"
                                fillGradient: LinearGradient {
                                    x1: 0; y1: 0
                                    x2: 0; y2: -20
                                    GradientStop { position: 0; color: "black" }
                                    GradientStop { position: 1; color: "red" }
                                }
                                PathLine { x: 0; y: 0 }
                                PathLine { x: 10; y: -20 }
                                PathLine { x: 20; y: 0 }
                            }
                        }
                    }

                    theme: GraphsTheme {
                        backgroundVisible: false
                        plotAreaBackgroundColor: "#00000000"
                        gridVisible: false
                        borderWidth: 0
                        labelBackgroundVisible: false
                        labelBorderVisible: false
                        colorScheme: Qt.Dark
                        theme: GraphsTheme.Theme.QtGreen
                        axisXLabelFont.pixelSize: 20
                        labelsVisible: false
                    }

                    axisX: BarCategoryAxis {
                        categories: ["Low", "Medium", "High"]
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

                    FrameAnimation {
                        id: barAnim
                        running: true
                        property real randomValue

                        onTriggered: {
                            let fn = (offset, speed) => {
                                return Math.sin (speed * elapsedTime + offset) + Math.sin(3.142 * (speed * elapsedTime + offset))
                            }

                            triangle1.x = 10 * fn(0, 1) + 50;
                            triangle2.x = 15 * fn(10, 2) + 50;
                            triangle3.x = 12 * fn(20, 0.3) + 50;
                            triangle4.x = 11 * fn(30, 1.3) + 50;
                        }
                    }
                    //! [1]
                    BarSeries {
                        property real barOpacity: 0.

                        id: barSeries
                        barsType: BarSeries.BarsType.Stacked
                        barWidth: 0.2

                        barDelegate: Item {
                            id: delegate
                            antialiasing: true
                            property real barOpacity: 0.5
                            property color barColor
                            property string barLabel

                            FrameAnimation {
                                running: true
                                onTriggered: {
                                    delegate.barOpacity = Math.abs(Math.sin(elapsedTime))
                                }
                            }
                            ShaderEffect {
                                id: effect
                                readonly property alias iTime: delegate.barOpacity
                                readonly property alias iColor: delegate.barColor
                                readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)

                                blending: true
                                fragmentShader: 'bar.frag.qsb'
                                anchors.fill: parent
                            }
                        }

                        BarSet { id: set1; label: "Low"; values: [1, 2, 3, 1]; color: "red" }
                        BarSet { id: set2; label: "Medium"; values: [2, 2, 0, 4]; color: "yellow"}
                        BarSet { id: set3; label: "High"; values: [3, 2, 3, 1]; color: "green"}
                    }
                    //! [1]
                }

                Loader { id: triangle1; sourceComponent: triangle; x: 50; y: 85 }
                Loader { id: triangle2; sourceComponent: triangle; x: 50; y: 174 }
                Loader { id: triangle3; sourceComponent: triangle; x: 50; y: 260 }
                Loader { id: triangle4; sourceComponent: triangle; x: 50; y: 350 }

                Component {
                    id: navigatorText

                    Text {
                        width: navColumn.Layout.preferredWidth
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: "white"
                        antialiasing: true
                        font.bold: true
                        font.family: "Akshar"
                        font.pointSize: 12
                    }
                }

                Loader { sourceComponent: navigatorText; onLoaded: { item.text = "OIL TEMP"; item.y = 30 } }
                Loader { sourceComponent: navigatorText; onLoaded: { item.text = "OIL PRES"; item.x = 0; item.y = 120 } }
                Loader { sourceComponent: navigatorText; onLoaded: { item.text = "COOLANT TEMP"; item.y = 210 } }
                Loader { sourceComponent: navigatorText; onLoaded: { item.text = "FUEL TEMP"; item.y = 300 } }
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: -40
                Layout.rightMargin: -25
                Layout.topMargin: -20
                Layout.bottomMargin: -20
                z: -1

                GraphsView {
                    id: mainNav
                    anchors.fill: parent
                    anchors.leftMargin: -50

                    theme: GraphsTheme {
                        backgroundVisible: false
                        plotAreaBackgroundColor: "#00000000"
                        gridVisible: true
                        borderWidth: 0
                        labelBackgroundVisible: false
                        labelBorderVisible: false
                        labelsVisible: false
                    }

                    // Crash with no axis here
                    axisX: ValueAxis {
                        visible: false
                        lineVisible: false
                        gridVisible: true
                        subGridVisible: false
                        id: xAxis
                        max: 10
                    }
                    axisY: ValueAxis {
                        visible: false
                        lineVisible: false
                        gridVisible: true
                        subGridVisible: false
                        id: yAxis
                        max: 10
                    }

                    //! [2]
                    AreaSeries {
                        property double x: 0
                        property double y: 0

                        id: lake1
                        color: "blue"
                        upperSeries: LineSeries {
                            id: s1
                            XYPoint { x: 0.0; y: -3.5 }
                            XYPoint { x: 1.0; y: -5.0 }
                            XYPoint { x: 2.0; y: -2.5 }
                            XYPoint { x: 2.5; y: -4.0 }
                            XYPoint { x: 3.0; y: -4.2 }
                        }

                        lowerSeries: LineSeries {
                            id: s2
                            XYPoint { x: 0.0; y: -7.2 }
                            XYPoint { x: 1.0; y: -7.0 }
                            XYPoint { x: 2.0; y: -8.5 }
                            XYPoint { x: 2.5; y: -8.0 }
                            XYPoint { x: 3.0; y: -9.0 }
                            XYPoint { x: 4.0; y: -6.5 }
                        }
                    }

                    AreaSeries {
                        property double x: 0
                        property double y: 0

                        id: lake2
                        color: "blue"
                        upperSeries: LineSeries {
                            id: s3
                            XYPoint { x: 0.0; y: 1.5 }
                            XYPoint { x: 1.0; y: 3.0 }
                            XYPoint { x: 2.0; y: 4.5 }
                            XYPoint { x: 2.5; y: 4.8 }
                            XYPoint { x: 3.0; y: 4.0 }
                        }

                        lowerSeries: LineSeries {
                            id: s4
                            XYPoint { x: 0.0; y: 0.0 }
                            XYPoint { x: 1.0; y: 0.5 }
                            XYPoint { x: 2.0; y: 0.2 }
                            XYPoint { x: 2.5; y: 1.5 }
                            XYPoint { x: 3.0; y: 1.0 }
                            XYPoint { x: 4.0; y: 0.6 }
                        }
                    }

                    // POI
                    ScatterSeries {
                        name: "Airport"

                        pointDelegate: Image {
                                source: "airplane-ico.png"
                                mipmap: true
                                width: 30
                                height: 30
                        }

                        XYPoint{x: 4.0; y: 5.7}
                        XYPoint{x: 2.2; y: 8.2}
                        XYPoint{x: 6.4; y: 1.2}
                        XYPoint{x: 7.4; y: 7.8}
                    }

                    LineSeries {
                        id: linePath
                        selectable: true
                        draggable: true
                        color: "white"
                        pointDelegate: Item {
                            width: 50
                            height: 50
                            property real pointValueX
                            property real pointValueY

                            FrameAnimation {
                                id: scatterAnim
                                running: true
                            }

                            ShaderEffect {
                                readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)
                                readonly property alias iTime: scatterAnim.elapsedTime

                                blending: true
                                fragmentShader: 'circleMarker.frag.qsb'
                                anchors.fill: parent
                            }

                            Text {
                                color: "white"
                                font.pointSize: 4
                                text: "LAT: " + pointValueX.toFixed(1) + ", " + "LON: " + pointValueY.toFixed(1)
                            }
                        }
                    }
                    //! [2]
                }
            }


            Rectangle {
                id: navControls
                color: "black"
                Layout.preferredWidth: root.width * 0.1
                Layout.preferredHeight: root.height
                border.color: "white"
                border.width: 2

                RowLayout {
                    anchors.fill: parent

                    Button {
                        property int yLoc: 1

                        Layout.leftMargin: 5
                        Layout.rightMargin: 5
                        Layout.fillWidth: true
                        text: "ADD"

                        onClicked: {
                            if (yLoc < 9)
                                linePath.append(5,yLoc++)
                        }
                    }
                }
            }
        }
    }
}
