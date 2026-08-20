// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick3D
import QtQuick3D.Helpers
import QtGraphs
import QtQml

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
Item {

    Gradient {
        id: customGradient
        GradientStop { id: redstop; position: 0.0; color: "red" }
        GradientStop { id: greenstop; position: 1.0; color: "green" }
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: cam

        environment: SceneEnvironment {
            clearColor: "skyblue"
            backgroundMode: SceneEnvironment.Color
        }

        DirectionalLight {
            eulerRotation.x: -30
        }

        PerspectiveCamera {
            id: cam
            property real xPos: Math.cos(camRot.value * 2 * Math.PI) * 100
            property real zPos: Math.sin(camRot.value * 2 * Math.PI) * 100
            position: Qt.vector3d(
                          xPos,
                          3,
                          zPos
                          )
            lookAtNode: lookAt
            eulerRotation.x: -30
            clipNear: 0.001
        }

        Surface3DNode {
            id: surface
            scale: Qt.vector3d(cSlider.value, cSlider.value, cSlider.value)
            position: Qt.vector3d(0,0, 4.0 * cSlider.value)
            polar: propertyButton.isEnabled
            flipHorizontalGrid: propertyButton.isEnabled
            margin: propertyButton.isEnabled? 0 : 0.1
            aspectRatio: 3

            theme: GraphsTheme {
                colorScheme: GraphsTheme.ColorScheme.Dark
                labelBorderVisible: true
                labelFont.pointSize: 35
                labelBackgroundVisible: true
                colorStyle: propertyButton.isEnabled? GraphsTheme.ColorStyle.RangeGradient : GraphsTheme.ColorStyle.Uniform
                baseGradients: [customGradient]
            }

            Surface3DSeries {
                id: heightSeries
                shading: Surface3DSeries.Shading.Smooth
                drawMode: propertyButton.isEnabled? Surface3DSeries.DrawSurface : Surface3DSeries.DrawSurfaceAndWireframe
                meshSmooth: true

                HeightMapSurfaceDataProxy {
                    heightMapFile: "://qml/qmlinjection/layer_1.png"
                    minYValue: 0
                    maxYValue: 7
                    autoScaleY: true
                }
            }
        }

        Bars3DNode {
            id: bars
            scale: Qt.vector3d(cSlider.value, cSlider.value, cSlider.value)
            optimizationHint: propertyButton.isEnabled? Graphs3D.OptimizationHint.Legacy : Graphs3D.OptimizationHint.Default
            theme: GraphsTheme {
                colorScheme: GraphsTheme.ColorScheme.Dark
                labelBorderVisible: true
                labelFont.pointSize: 35
                labelBackgroundVisible: true
                colorStyle: GraphsTheme.ColorStyle.RangeGradient
                baseGradients: [customGradient]
            }

            Bar3DSeries {
                mesh: Abstract3DSeries.Mesh.Cylinder
                ItemModelBarDataProxy {
                    itemModel: ListModel {
                        ListElement{ coords: "0,0"; data: "20.0/10.0/4.75"; }
                        ListElement{ coords: "1,0"; data: "21.1/10.3/3.00"; }
                        ListElement{ coords: "2,0"; data: "22.5/10.7/1.24"; }
                        ListElement{ coords: "3,0"; data: "24.0/10.5/2.53"; }
                        ListElement{ coords: "0,1"; data: "20.2/11.2/3.55"; }
                        ListElement{ coords: "1,1"; data: "21.3/11.5/3.03"; }
                        ListElement{ coords: "2,1"; data: "22.6/11.7/3.46"; }
                        ListElement{ coords: "3,1"; data: "23.4/11.5/4.12"; }
                        ListElement{ coords: "0,2"; data: "20.2/12.3/3.37"; }
                        ListElement{ coords: "1,2"; data: "21.1/12.4/2.98"; }
                        ListElement{ coords: "2,2"; data: "22.5/12.1/3.33"; }
                        ListElement{ coords: "3,2"; data: "23.3/12.7/3.23"; }
                        ListElement{ coords: "0,3"; data: "20.7/13.3/5.34"; }
                        ListElement{ coords: "1,3"; data: "21.5/13.2/4.54"; }
                        ListElement{ coords: "2,3"; data: "22.4/13.6/4.65"; }
                        ListElement{ coords: "3,3"; data: "23.2/13.4/6.67"; }
                        ListElement{ coords: "0,4"; data: "20.6/15.0/6.01"; }
                        ListElement{ coords: "1,4"; data: "21.3/14.6/5.83"; }
                        ListElement{ coords: "2,4"; data: "22.5/14.8/7.32"; }
                        ListElement{ coords: "3,4"; data: "23.7/14.3/6.90"; }
                    }
                    rowRole: "coords"
                    valueRole: "coords"
                    columnRole: "data"
                    rowRolePattern: /(\d),\d/
                    valueRolePattern: /(\d),(\d)/
                    columnRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    rowRoleReplace: "\\1"
                    valueRoleReplace: "\\2"
                    columnRoleReplace: "\\3"
                }
            }
        }

        Scatter3DNode {
            id: scatter
            scale: Qt.vector3d(cSlider.value, cSlider.value, cSlider.value)
            position: Qt.vector3d(0,0, -4.0 * cSlider.value)
            optimizationHint: propertyButton.isEnabled? Graphs3D.OptimizationHint.Legacy : Graphs3D.OptimizationHint.Default
            theme: GraphsTheme {
                colorScheme: GraphsTheme.ColorScheme.Dark
                labelBorderVisible: true
                labelFont.pointSize: 35
                labelBackgroundVisible: true
                colorStyle: GraphsTheme.ColorStyle.RangeGradient
                baseGradients: [customGradient]
            }

            Scatter3DSeries {
                ItemModelScatterDataProxy {
                    itemModel: ListModel {
                        ListElement{ coords: "0,0"; data: "20.0/10.0/4.75"; }
                        ListElement{ coords: "1,0"; data: "21.1/10.3/3.00"; }
                        ListElement{ coords: "2,0"; data: "22.5/10.7/1.24"; }
                        ListElement{ coords: "3,0"; data: "24.0/10.5/2.53"; }
                        ListElement{ coords: "0,1"; data: "20.2/11.2/3.55"; }
                        ListElement{ coords: "1,1"; data: "21.3/11.5/3.03"; }
                        ListElement{ coords: "2,1"; data: "22.6/11.7/3.46"; }
                        ListElement{ coords: "3,1"; data: "23.4/11.5/4.12"; }
                        ListElement{ coords: "0,2"; data: "20.2/12.3/3.37"; }
                        ListElement{ coords: "1,2"; data: "21.1/12.4/2.98"; }
                        ListElement{ coords: "2,2"; data: "22.5/12.1/3.33"; }
                        ListElement{ coords: "3,2"; data: "23.3/12.7/3.23"; }
                        ListElement{ coords: "0,3"; data: "20.7/13.3/5.34"; }
                        ListElement{ coords: "1,3"; data: "21.5/13.2/4.54"; }
                        ListElement{ coords: "2,3"; data: "22.4/13.6/4.65"; }
                        ListElement{ coords: "3,3"; data: "23.2/13.4/6.67"; }
                        ListElement{ coords: "0,4"; data: "20.6/15.0/6.01"; }
                        ListElement{ coords: "1,4"; data: "21.3/14.6/5.83"; }
                        ListElement{ coords: "2,4"; data: "22.5/14.8/7.32"; }
                        ListElement{ coords: "3,4"; data: "23.7/14.3/6.90"; }
                    }
                    zPosRole: "coords"
                    yPosRole: "coords"
                    xPosRole: "data"
                    zPosRolePattern: /(\d),\d/
                    yPosRolePattern: /(\d),(\d)/
                    xPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    zPosRoleReplace: "\\1"
                    yPosRoleReplace: "\\2"
                    xPosRoleReplace: "\\3"
                }
            }
        }


        Node {
            id: lookAt
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                bars.doPicking(Qt.point(mouseX, mouseY))
                scatter.doPicking(Qt.point(mouseX, mouseY))
                surface.doPicking(Qt.point(mouseX, mouseY))
            }
        }
    }

    Slider {
        anchors.bottom: view3d.bottom
        anchors.left: view3d.left
        anchors.right: view3d.right
        id: camRot
        from: 0
        to: 1
        stepSize: 0.01
        value: 0
    }

    RowLayout {

        Button {
            id: propertyButton
            property bool isEnabled: true
            text: "Change various properties"
            onClicked: isEnabled = !isEnabled
        }

        Text {
            text: "Graph scale"
        }

        Slider {
            id: cSlider
            from: 1
            value: 20
            to: 50
        }
    }
}
