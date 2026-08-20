// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtGraphs
import QtQuick
import QtQuick.Layouts
import "."
import QtQuick3D.Helpers
import QtQuick.Controls

Item {
    id: mainView
    width: 1080
    height: 720
    visible: true

    Rectangle {
        id: panels
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width / 5
        color: "green"

        TabBar {
            id: tabBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            contentHeight: 30
            TabButton {
                text: qsTr("Surface")
            }
            TabButton {
                text: qsTr("Scatter")
            }
            TabButton {
                text: qsTr("Bars")
            }
        }

        StackLayout {
            id: settingsStack
            anchors.top: tabBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            currentIndex: tabBar.currentIndex

            ListView {
                id: surfaceSettings
                ScrollBar.vertical: ScrollBar {}
                model: ObjectModel {
                    AxisSettings {
                        id: settings1
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        series: surfSeries1
                        index: 1
                    }
                    AxisSettings {
                        id: settings2
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        color: "orange"
                        series: surfSeries2
                        index: 2
                    }
                    AxisSettings {
                        id: settings3
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        color: "lightblue"
                        series: surfSeries3
                        index: 3
                    }
                }
            }
            ListView {
                id: scatterSettings
                ScrollBar.vertical: ScrollBar {}
                model: ObjectModel {
                    AxisSettings {
                        id: scatterSettings1
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        series: scatterSeries1
                        index: 1
                    }
                    AxisSettings {
                        id: scatterSettings2
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        color: "orange"
                        series: scatterSeries2
                        index: 2
                    }
                    AxisSettings {
                        id: scatterSettings3
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        color: "lightblue"
                        series: scatterSeries3
                        index: 3
                    }
                }
            }
            ListView {
                id: barSettings
                ScrollBar.vertical: ScrollBar {}
                model: ObjectModel {
                    BarAxisSettings {
                        id: barSettings1
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        series: barSeries1
                        index: 1
                    }
                    BarAxisSettings {
                        id: barSettings2
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        color: "orange"
                        series: barSeries2
                        index: 2
                    }
                    BarAxisSettings {
                        id: barSettings3
                        width: settingsStack.width
                        anchors.rightMargin: 15
                        color: "lightblue"
                        series: barSeries3
                        index: 3
                    }
                }
            }
        }
    }
    StackLayout {
        id: graphStack
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: panels.right
        anchors.right: parent.right
        currentIndex: tabBar.currentIndex

        Surface3D {
            id: surface
            polar: polarButton.isPolar
            orthoProjection: polarButton.isPolar

            Surface3DSeries {
                id: surfSeries1
                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/heightmap.jpg"
                    autoScaleY: true
                }
                shading: Surface3DSeries.Shading.Smooth
                drawMode: Surface3DSeries.DrawSurface
                baseColor: "white"
            }

            Surface3DSeries {
                id: surfSeries2
                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/heightmap.jpg"
                    autoScaleY: true
                }
                shading: Surface3DSeries.Shading.Smooth
                drawMode: Surface3DSeries.DrawSurface
                baseColor: "orange"
            }
            Surface3DSeries {
                id: surfSeries3
                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/heightmap.jpg"
                    autoScaleY: true
                }
                shading: Surface3DSeries.Shading.Smooth
                drawMode: Surface3DSeries.DrawSurface
                baseColor: "lightblue"
            }
        }

        Scatter3D {
            id: scatter
            polar: polarButton.isPolar
            orthoProjection: polarButton.isPolar

            Scatter3DSeries {
                id: scatterSeries1
                baseColor: "white"
                ItemModelScatterDataProxy {
                    itemModel: ListModel {
                        ListElement {
                            coords: "0,0"
                            data: "20.0/10.0/4.75"
                        }
                        ListElement {
                            coords: "1,0"
                            data: "21.1/10.3/3.00"
                        }
                        ListElement {
                            coords: "2,0"
                            data: "22.5/10.7/1.24"
                        }
                        ListElement {
                            coords: "3,0"
                            data: "24.0/10.5/2.53"
                        }
                        ListElement {
                            coords: "0,1"
                            data: "20.2/11.2/3.55"
                        }
                        ListElement {
                            coords: "1,1"
                            data: "21.3/11.5/3.03"
                        }
                        ListElement {
                            coords: "2,1"
                            data: "22.6/11.7/3.46"
                        }
                        ListElement {
                            coords: "3,1"
                            data: "23.4/11.5/4.12"
                        }
                        ListElement {
                            coords: "0,2"
                            data: "20.2/12.3/3.37"
                        }
                        ListElement {
                            coords: "1,2"
                            data: "21.1/12.4/2.98"
                        }
                        ListElement {
                            coords: "2,2"
                            data: "22.5/12.1/3.33"
                        }
                        ListElement {
                            coords: "3,2"
                            data: "23.3/12.7/3.23"
                        }
                        ListElement {
                            coords: "0,3"
                            data: "20.7/13.3/5.34"
                        }
                        ListElement {
                            coords: "1,3"
                            data: "21.5/13.2/4.54"
                        }
                        ListElement {
                            coords: "2,3"
                            data: "22.4/13.6/4.65"
                        }
                        ListElement {
                            coords: "3,3"
                            data: "23.2/13.4/6.67"
                        }
                        ListElement {
                            coords: "0,4"
                            data: "20.6/15.0/6.01"
                        }
                        ListElement {
                            coords: "1,4"
                            data: "21.3/14.6/5.83"
                        }
                        ListElement {
                            coords: "2,4"
                            data: "22.5/14.8/7.32"
                        }
                        ListElement {
                            coords: "3,4"
                            data: "23.7/14.3/6.90"
                        }
                    }
                    xPosRole: "coords"
                    zPosRole: "coords"
                    yPosRole: "data"
                    xPosRolePattern: /(\d),\d/
                    zPosRolePattern: /(\d),(\d)/
                    yPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    xPosRoleReplace: "\\1"
                    zPosRoleReplace: "\\2"
                    yPosRoleReplace: "\\3"
                }
            }

            Scatter3DSeries {
                id: scatterSeries2
                baseColor: "orange"
                ItemModelScatterDataProxy {
                    itemModel: ListModel {
                        ListElement {
                            coords: "0,0"
                            data: "20.0/10.0/4.75"
                        }
                        ListElement {
                            coords: "1,0"
                            data: "21.1/10.3/3.00"
                        }
                        ListElement {
                            coords: "2,0"
                            data: "22.5/10.7/1.24"
                        }
                        ListElement {
                            coords: "3,0"
                            data: "24.0/10.5/2.53"
                        }
                        ListElement {
                            coords: "0,1"
                            data: "20.2/11.2/3.55"
                        }
                        ListElement {
                            coords: "1,1"
                            data: "21.3/11.5/3.03"
                        }
                        ListElement {
                            coords: "2,1"
                            data: "22.6/11.7/3.46"
                        }
                        ListElement {
                            coords: "3,1"
                            data: "23.4/11.5/4.12"
                        }
                        ListElement {
                            coords: "0,2"
                            data: "20.2/12.3/3.37"
                        }
                        ListElement {
                            coords: "1,2"
                            data: "21.1/12.4/2.98"
                        }
                        ListElement {
                            coords: "2,2"
                            data: "22.5/12.1/3.33"
                        }
                        ListElement {
                            coords: "3,2"
                            data: "23.3/12.7/3.23"
                        }
                        ListElement {
                            coords: "0,3"
                            data: "20.7/13.3/5.34"
                        }
                        ListElement {
                            coords: "1,3"
                            data: "21.5/13.2/4.54"
                        }
                        ListElement {
                            coords: "2,3"
                            data: "22.4/13.6/4.65"
                        }
                        ListElement {
                            coords: "3,3"
                            data: "23.2/13.4/6.67"
                        }
                        ListElement {
                            coords: "0,4"
                            data: "20.6/15.0/6.01"
                        }
                        ListElement {
                            coords: "1,4"
                            data: "21.3/14.6/5.83"
                        }
                        ListElement {
                            coords: "2,4"
                            data: "22.5/14.8/7.32"
                        }
                        ListElement {
                            coords: "3,4"
                            data: "23.7/14.3/6.90"
                        }
                    }
                    xPosRole: "coords"
                    zPosRole: "coords"
                    yPosRole: "data"
                    xPosRolePattern: /(\d),\d/
                    zPosRolePattern: /(\d),(\d)/
                    yPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    xPosRoleReplace: "\\1"
                    zPosRoleReplace: "\\2"
                    yPosRoleReplace: "\\3"
                }
            }
            Spline3DSeries {
                id: scatterSeries3
                baseColor: "lightblue"
                ItemModelScatterDataProxy {
                    itemModel: ListModel {
                        ListElement {
                            coords: "0,0"
                            data: "20.0/10.0/4.75"
                        }
                        ListElement {
                            coords: "1,0"
                            data: "21.1/10.3/3.00"
                        }
                        ListElement {
                            coords: "2,0"
                            data: "22.5/10.7/1.24"
                        }
                        ListElement {
                            coords: "3,0"
                            data: "24.0/10.5/2.53"
                        }
                        ListElement {
                            coords: "0,1"
                            data: "20.2/11.2/3.55"
                        }
                        ListElement {
                            coords: "1,1"
                            data: "21.3/11.5/3.03"
                        }
                        ListElement {
                            coords: "2,1"
                            data: "22.6/11.7/3.46"
                        }
                        ListElement {
                            coords: "3,1"
                            data: "23.4/11.5/4.12"
                        }
                        ListElement {
                            coords: "0,2"
                            data: "20.2/12.3/3.37"
                        }
                        ListElement {
                            coords: "1,2"
                            data: "21.1/12.4/2.98"
                        }
                        ListElement {
                            coords: "2,2"
                            data: "22.5/12.1/3.33"
                        }
                        ListElement {
                            coords: "3,2"
                            data: "23.3/12.7/3.23"
                        }
                        ListElement {
                            coords: "0,3"
                            data: "20.7/13.3/5.34"
                        }
                        ListElement {
                            coords: "1,3"
                            data: "21.5/13.2/4.54"
                        }
                        ListElement {
                            coords: "2,3"
                            data: "22.4/13.6/4.65"
                        }
                        ListElement {
                            coords: "3,3"
                            data: "23.2/13.4/6.67"
                        }
                        ListElement {
                            coords: "0,4"
                            data: "20.6/15.0/6.01"
                        }
                        ListElement {
                            coords: "1,4"
                            data: "21.3/14.6/5.83"
                        }
                        ListElement {
                            coords: "2,4"
                            data: "22.5/14.8/7.32"
                        }
                        ListElement {
                            coords: "3,4"
                            data: "23.7/14.3/6.90"
                        }
                    }
                    xPosRole: "coords"
                    zPosRole: "coords"
                    yPosRole: "data"
                    xPosRolePattern: /(\d),\d/
                    zPosRolePattern: /(\d),(\d)/
                    yPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    xPosRoleReplace: "\\1"
                    zPosRoleReplace: "\\2"
                    yPosRoleReplace: "\\3"
                }
            }
        }

        Bars3D {
            floorLevel: floorLevelSlider.value
            Bar3DSeries {
                id: barSeries1
                baseColor: "white"
                ItemModelBarDataProxy {
                    itemModel: ListModel {
                        ListElement {
                            coords: "0,0"
                            data: "20.0/10.0/4.75"
                        }
                        ListElement {
                            coords: "1,0"
                            data: "21.1/10.3/3.00"
                        }
                        ListElement {
                            coords: "2,0"
                            data: "22.5/10.7/1.24"
                        }
                        ListElement {
                            coords: "3,0"
                            data: "24.0/10.5/2.53"
                        }
                        ListElement {
                            coords: "0,1"
                            data: "20.2/11.2/3.55"
                        }
                        ListElement {
                            coords: "1,1"
                            data: "21.3/11.5/3.03"
                        }
                        ListElement {
                            coords: "2,1"
                            data: "22.6/11.7/3.46"
                        }
                        ListElement {
                            coords: "3,1"
                            data: "23.4/11.5/4.12"
                        }
                        ListElement {
                            coords: "0,2"
                            data: "20.2/12.3/3.37"
                        }
                        ListElement {
                            coords: "1,2"
                            data: "21.1/12.4/2.98"
                        }
                        ListElement {
                            coords: "2,2"
                            data: "22.5/12.1/3.33"
                        }
                        ListElement {
                            coords: "3,2"
                            data: "23.3/12.7/3.23"
                        }
                        ListElement {
                            coords: "0,3"
                            data: "20.7/13.3/5.34"
                        }
                        ListElement {
                            coords: "1,3"
                            data: "21.5/13.2/4.54"
                        }
                        ListElement {
                            coords: "2,3"
                            data: "22.4/13.6/4.65"
                        }
                        ListElement {
                            coords: "3,3"
                            data: "23.2/13.4/6.67"
                        }
                        ListElement {
                            coords: "0,4"
                            data: "20.6/15.0/6.01"
                        }
                        ListElement {
                            coords: "1,4"
                            data: "21.3/14.6/5.83"
                        }
                        ListElement {
                            coords: "2,4"
                            data: "22.5/14.8/7.32"
                        }
                        ListElement {
                            coords: "3,4"
                            data: "23.7/14.3/6.90"
                        }
                    }
                    rowRole: "coords"
                    columnRole: "coords"
                    valueRole: "data"
                    rowRolePattern: /(\d),\d/
                    columnRolePattern: /(\d),(\d)/
                    valueRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    rowRoleReplace: "\\1"
                    columnRoleReplace: "\\2"
                    valueRoleReplace: "\\3"
                }
            }
            Bar3DSeries {
                id: barSeries2
                baseColor: "orange"
                ItemModelBarDataProxy {
                    itemModel: ListModel {
                        ListElement {
                            coords: "0,0"
                            data: "20.0/10.0/4.75"
                        }
                        ListElement {
                            coords: "1,0"
                            data: "21.1/10.3/3.00"
                        }
                        ListElement {
                            coords: "2,0"
                            data: "22.5/10.7/1.24"
                        }
                        ListElement {
                            coords: "3,0"
                            data: "24.0/10.5/2.53"
                        }
                        ListElement {
                            coords: "0,1"
                            data: "20.2/11.2/3.55"
                        }
                        ListElement {
                            coords: "1,1"
                            data: "21.3/11.5/3.03"
                        }
                        ListElement {
                            coords: "2,1"
                            data: "22.6/11.7/3.46"
                        }
                        ListElement {
                            coords: "3,1"
                            data: "23.4/11.5/4.12"
                        }
                        ListElement {
                            coords: "0,2"
                            data: "20.2/12.3/3.37"
                        }
                        ListElement {
                            coords: "1,2"
                            data: "21.1/12.4/2.98"
                        }
                        ListElement {
                            coords: "2,2"
                            data: "22.5/12.1/3.33"
                        }
                        ListElement {
                            coords: "3,2"
                            data: "23.3/12.7/3.23"
                        }
                        ListElement {
                            coords: "0,3"
                            data: "20.7/13.3/5.34"
                        }
                        ListElement {
                            coords: "1,3"
                            data: "21.5/13.2/4.54"
                        }
                        ListElement {
                            coords: "2,3"
                            data: "22.4/13.6/4.65"
                        }
                        ListElement {
                            coords: "3,3"
                            data: "23.2/13.4/6.67"
                        }
                        ListElement {
                            coords: "0,4"
                            data: "20.6/15.0/6.01"
                        }
                        ListElement {
                            coords: "1,4"
                            data: "21.3/14.6/5.83"
                        }
                        ListElement {
                            coords: "2,4"
                            data: "22.5/14.8/7.32"
                        }
                        ListElement {
                            coords: "3,4"
                            data: "23.7/14.3/6.90"
                        }
                    }
                    rowRole: "coords"
                    columnRole: "coords"
                    valueRole: "data"
                    rowRolePattern: /(\d),\d/
                    columnRolePattern: /(\d),(\d)/
                    valueRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    rowRoleReplace: "\\1"
                    columnRoleReplace: "\\2"
                    valueRoleReplace: "\\3"
                }
            }
            Bar3DSeries {
                id: barSeries3
                baseColor: "lightblue"
                ItemModelBarDataProxy {
                    itemModel: ListModel {
                        ListElement {
                            coords: "0,0"
                            data: "20.0/10.0/4.75"
                        }
                        ListElement {
                            coords: "1,0"
                            data: "21.1/10.3/3.00"
                        }
                        ListElement {
                            coords: "2,0"
                            data: "22.5/10.7/1.24"
                        }
                        ListElement {
                            coords: "3,0"
                            data: "24.0/10.5/2.53"
                        }
                        ListElement {
                            coords: "0,1"
                            data: "20.2/11.2/3.55"
                        }
                        ListElement {
                            coords: "1,1"
                            data: "21.3/11.5/3.03"
                        }
                        ListElement {
                            coords: "2,1"
                            data: "22.6/11.7/3.46"
                        }
                        ListElement {
                            coords: "3,1"
                            data: "23.4/11.5/4.12"
                        }
                        ListElement {
                            coords: "0,2"
                            data: "20.2/12.3/3.37"
                        }
                        ListElement {
                            coords: "1,2"
                            data: "21.1/12.4/2.98"
                        }
                        ListElement {
                            coords: "2,2"
                            data: "22.5/12.1/3.33"
                        }
                        ListElement {
                            coords: "3,2"
                            data: "23.3/12.7/3.23"
                        }
                        ListElement {
                            coords: "0,3"
                            data: "20.7/13.3/5.34"
                        }
                        ListElement {
                            coords: "1,3"
                            data: "21.5/13.2/4.54"
                        }
                        ListElement {
                            coords: "2,3"
                            data: "22.4/13.6/4.65"
                        }
                        ListElement {
                            coords: "3,3"
                            data: "23.2/13.4/6.67"
                        }
                        ListElement {
                            coords: "0,4"
                            data: "20.6/15.0/6.01"
                        }
                        ListElement {
                            coords: "1,4"
                            data: "21.3/14.6/5.83"
                        }
                        ListElement {
                            coords: "2,4"
                            data: "22.5/14.8/7.32"
                        }
                        ListElement {
                            coords: "3,4"
                            data: "23.7/14.3/6.90"
                        }
                    }
                    rowRole: "coords"
                    columnRole: "coords"
                    valueRole: "data"
                    rowRolePattern: /(\d),\d/
                    columnRolePattern: /(\d),(\d)/
                    valueRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    rowRoleReplace: "\\1"
                    columnRoleReplace: "\\2"
                    valueRoleReplace: "\\3"
                }
            }
        }
    }

    Button {
        id: polarButton
        visible: tabBar.currentIndex != 2
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 100
        height: 50

        property bool isPolar: false
        text: isPolar ? "Set cartesian" : "Set polar"
        onClicked: {
            isPolar = !isPolar;
            if (isPolar) {
                surface.cameraPreset = Graphs3D.CameraPreset.DirectlyAbove;
            } else {
                surface.cameraPreset = Graphs3D.CameraPreset.Front;
            }
        }
    }

    ColumnLayout {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 100
        height: 50

        Text {
            color: "white"
            text: qsTr("Floor Level: %1").arg(floorLevelSlider.value)
        }

        Slider {
            id: floorLevelSlider
            visible: tabBar.currentIndex = 2
            from: -10
            to: 10
            stepSize: 0.1
        }
    }
}
