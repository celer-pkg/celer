// Copyright (C) 2025 The Qt Company Ltd.performance
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import QtQml
import QtGraphs
import custom.model

Item {
    id: mainView
    width: 1080
    height: 720

    // RandomModel {
    //     id: randomModel
    //     rowCount: pointsSB.value
    // }

    RandomGenerator {
        id: randomGenerator
        rowCount: pointsSB.value
        surface3DSeries: surfaceSeries
        scatter3DSeries: scatter3DSeries
        spline3DSeries: spline3DSeries
        lineSeries: line2DSeries
        splineSeries: spline2DSeries
        scatterSeries: scatter2DSeries
        activeSeriesIndex: stackLayout.currentIndex
    }

    ListModel {
        id: phModel
    }

    //also doubles as fps counter
    FrameAnimation {
        id: fAnim
        running: true
        property real fps: smoothFrameTime > 0 ? (1.0 / smoothFrameTime) : 0
        // onTriggered: randomModel.nextCache()
        onTriggered: randomGenerator.nextCache()
    }

    TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        contentHeight: 30

        TabButton {
            text: qsTr("3D Surface")
        }

        TabButton {
            text: qsTr("3D Scatter")
        }

        TabButton {
            text: qsTr("3D Spline")
        }

        TabButton {
            text: qsTr("2D Lines")
        }

        TabButton {
            text: qsTr("2D Splines")
        }

        TabButton {
            text: qsTr("2D Scatter")
        }
    }

    StackLayout {
        id: stackLayout
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: tabBar.currentIndex

        Item {
            id: surfaceTab
            Surface3D {
                id: surface3D
                anchors.fill: parent
                orthoProjection: true
                cameraZoomLevel: 140
                axisY.min: 0
                axisY.max: 1

                theme : GraphsTheme {
                    theme: GraphsTheme.Theme.QtGreen
                }

                Surface3DSeries {
                    id: surfaceSeries
                    drawMode: Surface3DSeries.DrawSurface
                    // ItemModelSurfaceDataProxy {
                    //     itemModel: stackLayout.currentIndex == 0 ? randomModel : phModel
                    //     useModelCategories: true
                    //     xPosRole: "xData"
                    //     yPosRole: "yData"
                    // }
                }
            }
        }
        Item {
            id: scatterTab
            Scatter3D {
                id: scatterGraph
                anchors.fill: parent
                shadowQuality: Graphs3D.ShadowQuality.None
                orthoProjection: true
                cameraZoomLevel: 140
                axisY.min: 0
                axisY.max: 1

                theme : GraphsTheme {
                    id: scatterTheme
                    theme: GraphsTheme.Theme.QtGreen

                }
                Scatter3DSeries {
                    id: scatter3DSeries
                    itemSize: 0.02
                    mesh: Abstract3DSeries.Mesh.Point
                    // ItemModelScatterDataProxy {
                    //     itemModel: stackLayout.currentIndex == 1 ? randomModel : phModel
                    //     xPosRole: "xData"
                    //     yPosRole: "yData"
                    // }
                }
            }
        }
        Item {
            id: spline3DTab
            Scatter3D {
                id: spline3DGraph
                anchors.fill: parent
                shadowQuality: Graphs3D.ShadowQuality.None
                orthoProjection: true
                cameraZoomLevel: 140
                axisY.min: 0
                axisY.max: 1

                theme : GraphsTheme {
                    id: splineTheme
                    theme: GraphsTheme.Theme.QtGreen
                }

                Spline3DSeries {
                    id: spline3DSeries
                    itemSize: 0.001
                    splineVisible: true
                    splineResolution: 2
                    baseColor: "white"
                    splineColor: "green"
                    mesh: Abstract3DSeries.Mesh.Point

                    // ItemModelScatterDataProxy {
                    //     itemModel: stackLayout.currentIndex == 2 ? randomModel : phModel
                    //     xPosRole: "xData"
                    //     yPosRole: "yData"
                    //
                    // }
                }
            }
        }

        Item {
            id: line2Dtab
            GraphsView {
                id: line2D
                anchors.fill: parent

                axisX: ValueAxis {
                    max: pointsSB.value
                    labelDecimals: 1
                }
                axisY: ValueAxis {
                    max: 1
                    tickInterval: 1
                    subTickCount: 4
                    labelDecimals: 1
                }
                LineSeries {
                    id: line2DSeries
                    width: 4
                }
                // XYModelMapper  {
                //     first: 0
                //     count: pointsSB.value
                //     orientation: Qt.Vertical
                //     xSection: 0
                //     ySection: 1
                //     series: line2DSeries
                //     model: stackLayout.currentIndex == 3 ? randomModel : phModel
                // }
            }

        }
        Item {
            id: spline2Dtab
            GraphsView {
                id: spline2D
                anchors.fill: parent

                axisX: ValueAxis {
                    max: pointsSB.value
                    labelDecimals: 1
                }
                axisY: ValueAxis {
                    max: 1
                    tickInterval: 1
                    subTickCount: 4
                    labelDecimals: 1
                }
                SplineSeries {
                    id: spline2DSeries
                    width: 4
                }
                // XYModelMapper  {
                //     first: 0
                //     count: pointsSB.value
                //     orientation: Qt.Vertical
                //     xSection: 0
                //     ySection: 1
                //     series: spline2DSeries
                //     // model: stackLayout.currentIndex == 4 ? randomModel : phModel
                // }
            }

        }

        Item {
            id: scatter2Dtab
            GraphsView {
                id: scatter2D
                anchors.fill: parent

                axisX: ValueAxis {
                    max: pointsSB.value
                    labelDecimals: 1
                }
                axisY: ValueAxis {
                    max: 1
                    tickInterval: 1
                    subTickCount: 4
                    labelDecimals: 1
                }
                ScatterSeries {
                    id: scatter2DSeries
                }
                // XYModelMapper  {
                //     first: 0
                //     count: pointsSB.value
                //     orientation: Qt.Vertical
                //     xSection: 0
                //     ySection: 1
                //     series: scatter2DSeries
                //     model: stackLayout.currentIndex == 5 ? randomModel : phModel
                // }
            }
        }
    }

    Button {
        id: updateButton
        anchors.bottom: parent.bottom
        onClicked:  fAnim.running = !fAnim.running
        text: fAnim.running? "Pause" : "Update"
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: qsTr("FPS: %1").arg(fAnim.fps.toFixed(0))
        color: "red"
        font.pointSize: 24
    } 

    SpinBox {
        anchors.bottom: parent.bottom
        anchors.left: updateButton.right
        id: pointsSB
        from: 10
        to: 10000
        stepSize: 50
        value: 10
        editable: true
    }
}
