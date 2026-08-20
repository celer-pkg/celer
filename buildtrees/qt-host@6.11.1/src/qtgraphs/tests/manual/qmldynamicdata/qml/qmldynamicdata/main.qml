// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick.Controls
import QtQuick.Layouts
import "."

Item {
    id: mainView

    width: 1280
    height: 720
    visible: true

    ListModel {
        id: graphModel
        ListElement{ xPos: 0.0; yPos: 0.0; zPos: 0.0; rotation: "0.92388, 0.220942, 0.220942, 0.220942"}
        ListElement{ xPos: 1.0; yPos: 1.0; zPos: 1.0; rotation: "@45,1.0,1.0,1.0" }
    }

    Timer {
        id: dataTimer
        interval: 1
        running: true
        repeat: true
        property bool isIncreasing: true
        property real rotationAngle: 0

        function generateQuaternion() {
            return "@" + Math.random() * 360 + "," + Math.random() + "," + Math.random() + "," + Math.random()
        }

        function appendRow() {
            graphModel.append({"xPos": Math.random(),
                                  "yPos": Math.random(),
                                  "zPos": Math.random(),
                                  "rotation": generateQuaternion()
                              });
        }

        onTriggered: {
            rotationAngle = rotationAngle + 1
            scatterSeries.setMeshAxisAndAngle(Qt.vector3d(1,1,1), rotationAngle)
            if (isIncreasing) {
                appendRow()
                appendRow()
                appendRow()
                appendRow()
                appendRow()
                appendRow()
                appendRow()
                appendRow()
                appendRow()
                appendRow()
                if (graphModel.count > 5000) {
                    scatterGraph.theme.theme = GraphsTheme.Theme.QtGreenNeon;
                    isIncreasing = false;
                }
            } else {
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                graphModel.remove(Math.random() * (graphModel.count - 1));
                if (graphModel.count === 2) {
                    scatterGraph.theme.theme = GraphsTheme.Theme.YellowSeries;
                    isIncreasing = true;
                }
            }
        }
    }

    Color {
        id: dynamicColor
        ColorAnimation on color {
            from: "red"
            to: "yellow"
            duration: 5000
            loops: Animation.Infinite
        }
    }

    Item {
        id: dataView
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height - shadowToggle.height

        Scatter3D {
            id: scatterGraph
            width: dataView.width
            height: dataView.height
            theme: GraphsTheme {
                theme: GraphsTheme.Theme.QtGreen
                baseColors: [dynamicColor]
            }
            shadowQuality: Graphs3D.ShadowQuality.SoftMedium
            cameraYRotation: 30.0
            axisX.min: 0
            axisY.min: 0
            axisZ.min: 0
            axisX.max: 1
            axisY.max: 1
            axisZ.max: 1

            Scatter3DSeries {
                id: scatterSeries
                itemLabelFormat: "X:@xLabel Y:@yLabel Z:@zLabel"
                mesh: Abstract3DSeries.Mesh.Cube

                ItemModelScatterDataProxy {
                    itemModel: graphModel
                    xPosRole: "xPos"
                    yPosRole: "yPos"
                    zPosRole: "zPos"
                    rotationRole: "rotation"
                }
            }

            Component.onCompleted: {
                scatterGraph.unsetDefaultInputHandler();
            }
        }

        MouseArea {
            id: inputArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onWheel: (wheel)=> {
                // Adjust zoom level based on what zoom range we're in.
                var zoomLevel = scatterGraph.cameraZoomLevel;
                if (zoomLevel > 100)
                    zoomLevel += wheel.angleDelta.y / 12.0;
                else if (zoomLevel > 50)
                    zoomLevel += wheel.angleDelta.y / 60.0;
                else
                    zoomLevel += wheel.angleDelta.y / 120.0;
                if (zoomLevel > 500)
                    zoomLevel = 500;
                else if (zoomLevel < 10)
                    zoomLevel = 10;

                scatterGraph.cameraZoomLevel = zoomLevel;
            }
        }

        Timer {
            id: reselectTimer
            interval: 33
            running: true
            repeat: true
            onTriggered: {
                scatterGraph.scene.selectionQueryPosition = Qt.point(-1, -1);
                scatterGraph.scene.selectionQueryPosition = Qt.point(inputArea.mouseX, inputArea.mouseY);
            }
        }
    }

    NumberAnimation {
        id: cameraAnimationX
        loops: Animation.Infinite
        running: true
        target: scatterGraph
        property:"cameraXRotation"
        from: 0.0
        to: 360.0
        duration: 20000
    }


    SequentialAnimation {
        id: cameraAnimationY
        loops: Animation.Infinite
        running: true

        NumberAnimation {
            target: scatterGraph
            property:"cameraYRotation"
            from: 5.0
            to: 45.0
            duration: 9000
            easing.type: Easing.InOutSine
        }

        NumberAnimation {
            target: scatterGraph
            property:"cameraYRotation"
            from: 45.0
            to: 5.0
            duration: 9000
            easing.type: Easing.InOutSine
        }
    }

    RowLayout {
        id: buttonLayout

        anchors.left: parent.left
        anchors.right: parent.right

        Button {
            id: shadowToggle

            Layout.fillWidth: true
            text: "Hide Shadows"

            onClicked: {
                if (scatterGraph.shadowQuality === Graphs3D.ShadowQuality.None) {
                    scatterGraph.shadowQuality = Graphs3D.ShadowQuality.SoftMedium;
                    text = "Hide Shadows";
                } else {
                    scatterGraph.shadowQuality = Graphs3D.ShadowQuality.None;
                    text = "Show Shadows";
                }
            }
        }

        Button {
            id: cameraToggle
            Layout.fillWidth: true
            text: "Pause Camera"

            onClicked: {
                cameraAnimationX.paused = !cameraAnimationX.paused;
                cameraAnimationY.paused = cameraAnimationX.paused;
                if (cameraAnimationX.paused) {
                    text = "Animate Camera";
                } else {
                    text = "Pause Camera";
                }
            }
        }

        Button {
            id: dynamicDataToggle
            Layout.fillWidth: true
            text: "Stop dynamic data"

            onClicked: {
                dataTimer.running = !dataTimer.running

                if (dataTimer.running)
                    text = "Stop dynamic data";
                 else
                    text = "Start dynamic data";
            }
        }

        Button {
            id: autoSelectionToggle
            Layout.fillWidth: true
            text: "Stop autoselection"

            onClicked: {
                reselectTimer.running = !reselectTimer.running

                if (reselectTimer.running)
                    text = "Stop autoselection";
                 else
                    text = "Start autoselection";
            }
        }

        Button {
            id: seriesItemLabelToggle
            Layout.fillWidth: true
            text: "Hide series item label"

            onClicked: {
                scatterSeries.itemLabelVisible = !scatterSeries.itemLabelVisible

                if (scatterSeries.itemLabelVisible)
                    text = "Hide series item label";
                 else
                    text = "Show series item label";
            }
        }

        Button {
            id: exitButton
            Layout.fillWidth: true
            text: "Quit"
            onClicked: Qt.quit();
        }
    }
}
