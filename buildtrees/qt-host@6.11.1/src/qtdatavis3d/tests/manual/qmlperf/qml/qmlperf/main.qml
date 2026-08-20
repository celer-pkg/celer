// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick3D.Helpers
import QtQuick.Dialogs
import QtDataVisualization
import "."

Item {
    id: mainView
    width: 1280
    height: 1024

    TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.right: panels.left
        anchors.left: parent.left
        contentHeight: 50
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

    Rectangle {
        id: panels
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width / 5
        color: "green"

        ColumnLayout {
            id: buttonPanel
            anchors.top: parent.top
            width: parent.width
            spacing: 10
            visible: autoTest.finished

            Button {
                id: shadowToggle
                property int shadowQuality: 0
                property string qualityName: "None"
                text: qsTr("Shadow Quality : %1").arg(qualityName)
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter
                onClicked: {
                    var nextQuality = (shadowQuality + 1) % 7
                    surfaceGraph.shadowQuality = nextQuality
                    scatterGraph.shadowQuality = nextQuality
                    barGraph.shadowQuality = nextQuality
                    shadowQuality = nextQuality
                    qualityName = barGraph.shadowQuality.toString()
                    console.log("Set shadow quality to " + qualityName)
                }
            }

            Button {
                id: optimizationToggle
                visible: tabBar.currentIndex > 0
                property string optimization: "Default"
                text: qsTr("Optimization: %1").arg(optimization)
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter
                onClicked: {
                    if (optimization === "Static") {
                        scatterGraph.optimizationHints = AbstractGraph3D.OptimizationDefault
                        barGraph.optimizationHints = AbstractGraph3D.OptimizationDefault
                        optimization= "Default"
                    } else {
                        scatterGraph.optimizationHints = AbstractGraph3D.OptimizationStatic
                        barGraph.optimizationHints = AbstractGraph3D.OptimizationStatic
                        optimization = "Static"
                    }
                    console.log("Set optimization to " + optimization)
                }
            }

            Button {
                id: samplesButton
                property list<int> samples: [0,2,4,8]
                property int index: 0
                text: qsTr("MSAA samples: %1").arg(samples[index])

                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter
                onClicked: {
                    index = (index + 1) % 4
                    surfaceGraph.msaaSamples = samples[index]
                    scatterGraph.msaaSamples = samples[index]
                    barGraph.msaaSamples = samples[index]
                    console.log("Set msaa samples to " + samples[index])
                }
            }

            Button {
                id: scatterMesh
                visible: tabBar.currentIndex === 1
                property string mesh: "Sphere"
                text: qsTr("Mesh: %1").arg(mesh)
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter
                onClicked: {
                    if (mesh === "Sphere") {
                        scatterSeries.mesh = Abstract3DSeries.MeshCube
                        mesh = "Cube"
                    } else if (mesh === "Cube") {
                        scatterSeries.mesh = Abstract3DSeries.MeshPyramid
                        mesh = "Pyramid"
                    } else if (mesh === "Pyramid") {
                        scatterSeries.mesh = Abstract3DSeries.MeshPoint
                        mesh = "Point"
                    } else {
                        scatterSeries.mesh = Abstract3DSeries.MeshSphere
                        mesh = "Sphere"
                        }
                }
            }


            Button {
                id: surfaceShadingToggle
                visible: tabBar.currentIndex <= 2
                text: qsTr("Flat shading: %1").arg(surfaceSeries.flatShadingEnabled.toString())
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter
                onClicked: {
                    surfaceSeries.flatShadingEnabled =
                            !surfaceSeries.flatShadingEnabled
                    scatterSeries.meshSmooth = !scatterSeries.meshSmooth
                }
            }

            Button {
                id: gridToggle
                visible: tabBar.currentIndex === 0
                property bool gridEnabled
                text: qsTr("Show grid: %1").arg(gridEnabled.toString())
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter
                onClicked: {
                    if (surfaceSeries.drawMode & Surface3DSeries.DrawWireframe)
                        surfaceSeries.drawMode &= ~Surface3DSeries.DrawWireframe;
                    else
                        surfaceSeries.drawMode |= Surface3DSeries.DrawWireframe;

                    gridEnabled = surfaceSeries.drawMode & Surface3DSeries.DrawWireframe
                }

            }

            ColumnLayout {
                id: pointSetContainer
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter


                Text {
                    id: spinboxTitle
                    text: "Side length"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                SpinBox {
                    id: sizeField
                    from: 2
                    to: 500
                    stepSize: 20
                    value: 10
                    editable: true
                    Layout.preferredWidth: parent.width
                }

                Button {
                    id: pointSetButton
                    text: qsTr("Place points");
                    Layout.preferredWidth: parent.width
                    Layout.bottomMargin: 10
                    onClicked: {
                        switch (tabBar.currentIndex) {
                        case 0:
                            dataGenerator.generateSurfaceData(surfaceSeries, sizeField.value)
                            break;
                        case 1:
                            dataGenerator.generateScatterData(scatterSeries, sizeField.value)
                            break;
                        case 2:
                            dataGenerator.generateBarData(barSeries, sizeField.value)
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }

        GridLayout {
            id: checkBoxPanel
            anchors.top: buttonPanel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            visible: autoTest.finished
            columns: 2

            CheckBox {
                id: liveDataCB
                text: qsTr("Live Data")
                Layout.fillWidth: true
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter
            }
            CheckBox {
                id: rotateCB
                text: qsTr("Rotation")
                Layout.fillWidth: true
                Layout.margins: 10
                Layout.alignment: Qt.AlignCenter
            }

            ColumnLayout {
                id: freqContainer
                Layout.columnSpan: 2
                Text {
                    text: qsTr("Frequency: %1").arg(frequencySlider.value)
                    Layout.fillWidth: true
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
                Slider {
                    id: frequencySlider
                    from: 1
                    to: 60
                    stepSize: 1
                    value: 30
                    snapMode: Slider.SnapAlways
                    Layout.alignment: Qt.AlignCenter
                }
            }
        }

        AutoTest {
            id: autoTest
            width: parent.width
            anchors.top: checkBoxPanel.bottom
            anchors.left: parent.left
            anchors.topMargin: 10
        }

        Tests {
            id: tests
            width: parent.width
            anchors.top: autoTest.bottom
            anchors.left: parent.left
            buttonVisible: autoTest.finished
        }

        Button {
            id: pathButton
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            text: "Set logfile path"
            anchors.margins: 10
            onClicked: folderDialog.open()
        }
    }

    FolderDialog {
        id: folderDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.DocumentsLocation)[0]
        onAccepted: dataGenerator.setFilePath(currentFolder)
    }

    Timer {
        id: rotationTimer
        interval: 15
        running: rotateCB.checked
        repeat: true
        onTriggered: {
            switch (tabBar.currentIndex) {
            case 0:
                if (++surfaceGraph.scene.activeCamera.xRotation == 360)
                    surfaceGraph.cameraXRotation = 0;
                break
            case 1:
                if (++scatterGraph.scene.activeCamera.xRotation == 360)
                    scatterGraph.cameraXRotation = 0;
                break
            case 2:
                if (++barGraph.scene.activeCamera.xRotation == 360)
                    barGraph.cameraXRotation = 0;
                break
            }
        }
    }

    Timer {
        id: updateTimer
        interval: 1000 / frequencySlider.value
        running: liveDataCB.checked
        repeat: true
        onTriggered: {
            switch (tabBar.currentIndex) {
            case 0:
                dataGenerator.updateSurfaceData(surfaceSeries)
                break
            case 1:
                dataGenerator.updateScatterData(scatterSeries)
                break
            case 2:
                dataGenerator.updateBarData(barSeries)
                break
            }
        }
    }


    StackLayout {
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: panels.left
        currentIndex: tabBar.currentIndex

        Item {
            id: surfaceTab
            Surface3D {
                id: surfaceGraph
                anchors.fill: parent
                shadowQuality: AbstractGraph3D.ShadowQualityNone
                scene.activeCamera.yRotation: 45.0
                measureFps: true

                axisX.min: 0
                axisX.max: 1
                axisY.min: 0
                axisY.max: 1
                horizontalAspectRatio: 1.0


                theme : Theme3D {
                    type: Theme3D.ThemeQt
                    colorStyle: Theme3D.ColorStyleRangeGradient
                    baseGradients: surfaceGradient

                    ColorGradient {
                        id: surfaceGradient
                        ColorGradientStop { position: 1.0; color: "red" }
                        ColorGradientStop { position: 0.0; color: "blue" }
                    }
                }

                Surface3DSeries {
                    id: surfaceSeries
                    dataProxy.onArrayReset: tests.dataPoints = dataProxy.columnCount * dataProxy.rowCount
                }

                onCurrentFpsChanged: {
                    tests.currentFps = currentFps
                }
            }
        }
        Item {
            id: scatterTab
            Scatter3D {
                id: scatterGraph
                anchors.fill: parent
                shadowQuality: AbstractGraph3D.ShadowQualityNone
                scene.activeCamera.yRotation: 45.0
                aspectRatio: 1.0
                horizontalAspectRatio: 1.0

                axisY.min: -1
                axisY.max: 1
                axisX.min: -1
                axisX.max: 1
                axisZ.min: -1
                axisZ.max: 1

                measureFps: true
                theme : Theme3D {
                    type: Theme3D.ThemeQt
                    colorStyle: Theme3D.ColorStyleRangeGradient
                    baseGradients: scatterGradient

                    ColorGradient {
                        id: scatterGradient
                        ColorGradientStop { position: 1.0; color: "yellow" }
                        ColorGradientStop { position: 0.6; color: "red" }
                        ColorGradientStop { position: 0.4; color: "blue" }
                        ColorGradientStop { position: 0.0; color: "green" }
                    }
                }
                Scatter3DSeries {
                    id: scatterSeries
                    dataProxy.onArrayReset: tests.dataPoints = dataProxy.itemCount
                    itemSize: 0.1
                }
                onCurrentFpsChanged: {
                    tests.currentFps = currentFps
                }
            }
        }
        Item {
            id: barTab
            Bars3D {
                id: barGraph
                anchors.fill: parent
                shadowQuality: AbstractGraph3D.ShadowQualityNone
                scene.activeCamera.yRotation: 45.0
                measureFps: true
                valueAxis.min: 0
                valueAxis.max: 1

                theme : Theme3D {
                    type: Theme3D.ThemeQt
                    colorStyle: Theme3D.ColorStyleRangeGradient
                    baseGradients: barGradient

                    ColorGradient{
                        id: barGradient
                        ColorGradientStop { position: 1.0; color: "red" }
                        ColorGradientStop { position: 0.0; color: "blue" }
                    }
                }

                Bar3DSeries {
                    id: barSeries
                    dataProxy.onArrayReset: tests.dataPoints
                                            = dataProxy.colCount * dataProxy.rowCount

                }

                onCurrentFpsChanged: {
                    tests.currentFps = currentFps
                }
            }
        }
    }
}
