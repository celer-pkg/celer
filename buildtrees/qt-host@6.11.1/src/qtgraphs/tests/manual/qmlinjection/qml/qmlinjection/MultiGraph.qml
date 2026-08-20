// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts

import QtQuick3D
import QtQuick3D.Helpers
import QtGraphs
import QtQml

Item {
    Scatter3D {
        id: scatter
        importScene: surf.rootNode
        anchors.fill: parent

        lightStrength: 0
        ambientLightStrength: 0
        theme: GraphsTheme {
            id: graphTheme
            colorScheme: GraphsTheme.ColorScheme.Dark
            labelFont.pointSize: 7
            plotAreaBackgroundVisible: false
            gridVisible: false
            labelsVisible: false
            backgroundColor: "skyblue"
        }

        axisY.max: 100
        axisX.max: 10
        axisZ.max: 10
        axisX.min: -1
        axisZ.min: -1
        Spline3DSeries {
            id: scatterSeries1
            splineVisible: true
            splineColor: "red"
            itemSize: 0.025
            baseColor: "white"
            itemLabelFormat: "Melbourne to Port Hedland International"
            userDefinedMesh: ":/qml/qmlinjection/plane.mesh"
            mesh: Abstract3DSeries.Mesh.UserDefined
            meshRotation: Quaternion.fromAxisAndAngle(Qt.vector3d(0,1,0), -125)
            ItemModelScatterDataProxy {
                itemModel: ListModel {
                    ListElement {x: "7.82"; z:"1.84"; height:"0.94";}
                    ListElement {x: "4.48"; z:"4.88"; height:"17.0";}
                    ListElement {x: "1.30"; z:"6.65"; height:"0.24";}
                }
                xPosRole: "x"
                zPosRole: "z"
                yPosRole: "height"
            }
        }
        Spline3DSeries {
            id: scatterSeries2
            splineVisible: true
            splineColor: "red"
            itemSize: 0.025
            baseColor: "white"
            itemLabelFormat: "Hobart International to Cairns"
            userDefinedMesh: ":/qml/qmlinjection/plane.mesh"
            mesh: Abstract3DSeries.Mesh.UserDefined
            meshRotation: Quaternion.fromAxisAndAngle(Qt.vector3d(0,1,0), 185)
            ItemModelScatterDataProxy {
                itemModel: ListModel {
                    ListElement {x: "8.39"; z:"0.60"; height:"3.33";}
                    ListElement {x: "8.05"; z:"4.97"; height:"15.0";}
                    ListElement {x: "8.07"; z:"7.57"; height:"0.82";}
                }
                xPosRole: "x"
                zPosRole: "z"
                yPosRole: "height"
            }
        }
        Spline3DSeries {
            id: scatterSeries3
            splineVisible: true
            splineColor: "red"
            itemSize: 0.025
            baseColor: "white"
            itemLabelFormat: "Perth to Darwin International"
            userDefinedMesh: ":/qml/qmlinjection/plane.mesh"
            mesh: Abstract3DSeries.Mesh.UserDefined
            meshRotation: Quaternion.fromAxisAndAngle(Qt.vector3d(0,1,0), 145)
            ItemModelScatterDataProxy {
                itemModel: ListModel {
                    ListElement {x: "0.76"; z:"3.28"; height:"1.84";}
                    ListElement {x: "2.80"; z:"5.75"; height:"13.0";}
                    ListElement {x: "4.45"; z:"8.82"; height:"0.98";}
                }
                xPosRole: "x"
                zPosRole: "z"
                yPosRole: "height"
            }
        }
    }

    Gradient {
        id: surfGradient
        GradientStop {position: 0.0; color: "darkblue"}
        GradientStop {position: 0.1; color: "forestgreen"}
        GradientStop {position: 0.15; color: "darkorange"}
        GradientStop {position: 1.0; color: "palegoldenrod"}
    }

   Surface3D {
       id: surf
       width: surfOnTop.checked? parent.width : 0
       height: surfOnTop.checked? parent.height : 0
       importScene: lightNode

       lightStrength: 0
       ambientLightStrength: 0
       axisY.max: 100
       axisX.max: 10
       axisZ.max: 10
       axisX.min: -1
       axisZ.min: -1

       theme: GraphsTheme {
           colorScheme: GraphsTheme.ColorScheme.Dark
           labelFont.pointSize: 7
           colorStyle: GraphsTheme.ColorStyle.ObjectGradient
           baseGradients: [surfGradient] // Use the custom gradient
           plotAreaBackgroundVisible: false
           gridVisible: false
           labelsVisible: false
           backgroundColor: "skyblue"
       }


       Surface3DSeries {
           id: heightSeries
           shading: Surface3DSeries.Shading.Smooth
           drawMode: Surface3DSeries.DrawSurface
           meshSmooth: true
           textureFile: "://qml/qmlinjection/australiaSatellite.jpg"

           HeightMapSurfaceDataProxy {
               heightMapFile: "://qml/qmlinjection/australia.png"
               minYValue: 0
               maxYValue: 7
               autoScaleY: true
           }
       }
   }

   Node {
       id: lightNode
       DirectionalLight {
           eulerRotation.x: -20
           eulerRotation.y: 90
           ambientColor: "navy"
       }
   }

   RowLayout {
       Text {
           text : "Surface graph on top"
       }
       CheckBox {
           id: surfOnTop
       }
   }
}
