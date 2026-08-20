// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtDataVisualization 1.2
import "."

Item {
    id: mainwindow

    function updateinfoLabels()
    {
        if (surfaceGraph.theme.baseGradients[0] === mainGradient)
            gradientLabel.text = "Main gradient";
        else if (surfaceGraph.theme.baseGradients[0] === secondaryGradient)
            gradientLabel.text = "Secondary gradient";
    }

    width: 1024
    height: 768
    visible: true

    Item {
        id: surfaceview
        width: mainwindow.width
        height: mainwindow.height

        anchors.top: mainwindow.top
        anchors.left: mainwindow.left

        ColorGradient {
            id: mainGradient
            ColorGradientStop { position: 0.0; color: "red"}
            ColorGradientStop { position: 0.5; color: "green"}
            ColorGradientStop { position: 0.8; color: "blue"}
            ColorGradientStop { position: 0.6; color: "yellow"}
            ColorGradientStop { position: 0.8; color: "black"}
            ColorGradientStop { position: 1.0; color: "peru"}
        }

        ColorGradient {
            id: secondaryGradient
            ColorGradientStop { position: 0.0; color: "crimson"}
            ColorGradientStop { position: 0.5; color: "chartreuse"}
            ColorGradientStop { position: 0.8; color: "blueviolet"}
            ColorGradientStop { position: 0.6; color: "gold"}
            ColorGradientStop { position: 0.8; color: "darkslategrey"}
            ColorGradientStop { position: 1.0; color: "seagreen"}
        }

        ColorGradient {
            id: seriesGradient
            ColorGradientStop { position: 0.0; color: "gold" }
            ColorGradientStop { position: 0.5; color: "crimson" }
            ColorGradientStop { position: 1.0; color: "blueviolet" }
        }

        Theme3D {
            id: mainTheme
            type: Q3DTheme.ThemeStoneMoss

            colorStyle: Q3DTheme.ColorStyleRangeGradient
            baseGradients: [mainGradient]
        }

        Theme3D {
            id: secondaryTheme
            type: Q3DTheme.ThemeArmyBlue
            baseGradients: [secondaryGradient]
        }

        Surface3D {
            id: surfaceGraph
            width: surfaceview.width
            height: surfaceview.height
            theme: mainTheme

            shadowQuality: AbstractGraph3D.ShadowQualityMedium
            selectionMode: AbstractGraph3D.SelectionSlice | AbstractGraph3D.SelectionItemAndRow
            scene.activeCamera.cameraPreset: Camera3D.CameraPresetIsometricLeft
            axisY.min: 0.0
            axisY.max: 500.0
            axisX.segmentCount: 10
            axisX.subSegmentCount: 2
            axisX.labelFormat: "%i"
            axisZ.segmentCount: 10
            axisZ.subSegmentCount: 2
            axisZ.labelFormat: "%i"
            axisY.segmentCount: 5
            axisY.subSegmentCount: 2
            axisY.labelFormat: "%i"
            axisY.title: "Height"
            axisX.title: "Latitude"
            axisZ.title: "Longitude"

            Surface3DSeries {
                id: heightSeries
                drawMode: Surface3DSeries.DrawSurface
                visible: true
                flatShadingEnabled: false

                HeightMapSurfaceDataProxy {
                    heightMapFile: ":/map"
                }
            }
        }

        RowLayout {
            id: buttonLayout
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            Button {
                id: toggleTheme
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: qsTr("Toggle theme")
                onClicked: {
                    if (surfaceGraph.theme == mainTheme) {
                        surfaceGraph.theme = secondaryTheme;
                        themeLabel.text = "Secondary theme";
                        updateinfoLabels();
                    } else if (surfaceGraph.theme == secondaryTheme) {
                        surfaceGraph.theme = mainTheme;
                        updateinfoLabels();
                        themeLabel.text = "Main theme";
                    }
                }
            }

            Button {
                id: toggleGradient
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: qsTr("Toggle theme gradient")
                onClicked: {
                    if (surfaceGraph.theme.baseGradients[0] === mainGradient) {
                        surfaceGraph.theme.baseGradients[0] = secondaryGradient;
                        updateinfoLabels();
                    } else if (surfaceGraph.theme.baseGradients[0] === secondaryGradient) {
                        surfaceGraph.theme.baseGradients[0] = mainGradient;
                        updateinfoLabels();
                    }
                }
            }

            Button {
                id: toggleSeriesGradient
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: qsTr("Override theme gradient with series gradient")

                onClicked: {
                    heightSeries.baseGradient = seriesGradient;
                    gradientLabel.text = "Series gradient";
                }
            }
        }

        ColumnLayout {
            id: infoLayout
            anchors.top: buttonLayout.bottom
            anchors.left: parent.left

            Rectangle {
                Layout.minimumHeight: 20

                Label {
                    id: themeLabel
                    text: qsTr("Main theme")
                }
            }

            Rectangle {
                Layout.minimumHeight: 20

                Label {
                    id: gradientLabel
                    text: qsTr("Main gradient")
                }
            }
        }
    }
}
