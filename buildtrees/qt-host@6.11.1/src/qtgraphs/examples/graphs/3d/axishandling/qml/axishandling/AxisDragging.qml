// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtGraphs

Item {
    id: axisDragView

    property int selectedAxisLabel: -1
    property real dragSpeedModifier: 100.0
    property int currentMouseX: -1
    property int currentMouseY: -1
    property int previousMouseX: -1
    property int previousMouseY: -1

    required property bool portraitMode

    ListModel {
        id: graphModel
        ListElement{ xPos: 0.0; yPos: 0.0; zPos: 0.0; rotation: "@0,0,0,0" }
        ListElement{ xPos: 1.0; yPos: 1.0; zPos: 1.0; rotation: "@45,1,1,1" }
    }

    Timer {
        id: dataTimer
        interval: 1
        running: true
        repeat: true
        property bool isIncreasing: true
        property real rotationAngle: 0

        function generateQuaternion() {
            return "@" + Math.random() * 360 + "," + Math.random() + ","
                    + Math.random() + "," + Math.random();
        }

        function appendRow() {
            graphModel.append({"xPos": Math.random(),
                                  "yPos": Math.random(),
                                  "zPos": Math.random(),
                                  "rotation": generateQuaternion()
                              });
        }

        //! [10]
        onTriggered: {
            rotationAngle = rotationAngle + 1;
            qtCube.setRotationAxisAndAngle(Qt.vector3d(1, 0, 1), rotationAngle);
            //! [10]
            scatterSeries.setMeshAxisAndAngle(Qt.vector3d(1, 1, 1), rotationAngle);
            if (isIncreasing) {
                for (var i = 0; i < 10; i++)
                    appendRow();
                if (graphModel.count > 2002) {
                    scatterGraph.theme = orangeTheme;
                    isIncreasing = false;
                }
            } else {
                graphModel.remove(2, 10);
                if (graphModel.count === 2) {
                    scatterGraph.theme = dynamicColorTheme;
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
            duration: 2000
            loops: Animation.Infinite
        }
    }

    GraphsTheme {
        id: dynamicColorTheme
        theme: GraphsTheme.Theme.QtGreen
        colorScheme: GraphsTheme.ColorScheme.Dark
        baseColors: [dynamicColor]
        labelFont.pointSize: 50
        labelBorderVisible: true
        labelBackgroundColor: "gold"
        labelTextColor: "black"
        axisX.labelTextColor: "black"
        axisY.labelTextColor: "black"
        axisZ.labelTextColor: "black"
    }

    GraphsTheme {
        id: orangeTheme
        theme: GraphsTheme.Theme.OrangeSeries
        colorScheme: GraphsTheme.ColorScheme.Dark
        labelFont.pointSize: 50
        labelBorderVisible: true
        labelBackgroundColor: "gold"
        labelTextColor: "black"
        axisX.labelTextColor: "black"
        axisY.labelTextColor: "black"
        axisZ.labelTextColor: "black"
    }

    Scatter3D {
        id: scatterGraph
        anchors.fill: parent
        theme: dynamicColorTheme
        shadowQuality: Graphs3D.ShadowQuality.Medium
        cameraYRotation: 45.0
        cameraXRotation: 45.0
        cameraZoomLevel: 75.0

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
        //! [9]
        customItemList: [
            Custom3DItem {
                id: qtCube
                meshFile: ":/qml/axishandling/cube.mesh"
                textureFile: ":/qml/axishandling/cubetexture.png"
                position: Qt.vector3d(0.65, 0.35, 0.65)
                scaling: Qt.vector3d(0.3, 0.3, 0.3)
            }
        ]
        //! [9]
        //! [5]
        onSelectedElementChanged: {
            if (selectedElement >= Graphs3D.ElementType.AxisXLabel
                    && selectedElement <= Graphs3D.ElementType.AxisZLabel) {
                selectedAxisLabel = selectedElement;
            } else {
                selectedAxisLabel = -1;
            }
        }
        //! [5]

        Component.onCompleted: {
            //! [0]
            unsetDefaultInputHandler();
            //! [0]
        }
    }

    //! [1]
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        //! [1]

        //! [3]
        onPositionChanged: (mouse)=> {
                               currentMouseX = mouse.x;
                               currentMouseY = mouse.y;
                               //! [3]
                               //! [6]
                               if (pressed && selectedAxisLabel != -1)
                                   axisDragView.dragAxis();
                               //! [6]
                               //! [4]
                               previousMouseX = currentMouseX;
                               previousMouseY = currentMouseY;
                           }
        //! [4]

        //! [2]
        onPressed: (mouse)=> {
                       scatterGraph.doPicking(Qt.point(mouse.x, mouse.y));
                   }
        //! [2]

        onReleased: {
            // We need to clear mouse positions and selected axis, because touch devices cannot
            // track position all the time
            selectedAxisLabel = -1;
            currentMouseX = -1;
            currentMouseY = -1;
            previousMouseX = -1;
            previousMouseY = -1;
        }
    }

    //! [7]
    function dragAxis() {
        // Do nothing if previous mouse position is uninitialized
        if (previousMouseX === -1)
            return;

        // Directional drag multipliers based on rotation. Camera is locked to 45 degrees, so we
        // can use one precalculated value instead of calculating xx, xy, zx and zy individually
        var cameraMultiplier = 0.70710678;

        // Calculate the mouse move amount
        var moveX = currentMouseX - previousMouseX;
        var moveY = currentMouseY - previousMouseY;

        // Adjust axes
        switch (selectedAxisLabel) {
        case Graphs3D.ElementType.AxisXLabel:
            var distance = ((moveX - moveY) * cameraMultiplier) / dragSpeedModifier;
            // Check if we need to change min or max first to avoid invalid ranges
            if (distance > 0) {
                scatterGraph.axisX.min -= distance;
                scatterGraph.axisX.max -= distance;
            } else {
                scatterGraph.axisX.max -= distance;
                scatterGraph.axisX.min -= distance;
            }
            break;
        case Graphs3D.ElementType.AxisYLabel:
            distance = moveY / dragSpeedModifier;
            // Check if we need to change min or max first to avoid invalid ranges
            if (distance > 0) {
                scatterGraph.axisY.max += distance;
                scatterGraph.axisY.min += distance;
            } else {
                scatterGraph.axisY.min += distance;
                scatterGraph.axisY.max += distance;
            }
            break;
        case Graphs3D.ElementType.AxisZLabel:
            distance = ((moveX + moveY) * cameraMultiplier) / dragSpeedModifier;
            // Check if we need to change min or max first to avoid invalid ranges
            if (distance > 0) {
                scatterGraph.axisZ.max += distance;
                scatterGraph.axisZ.min += distance;
            } else {
                scatterGraph.axisZ.min += distance;
                scatterGraph.axisZ.max += distance;
            }
            break;
        }
    }
    //! [7]

    Button {
        id: rangeToggle
        // We're adding 3 buttons and want to divide them equally, if not in portrait mode
        width: axisDragView.portraitMode ? parent.width : parent.width / 3
        text: "Use Preset Range"
        anchors.left: parent.left
        anchors.top: parent.top
        property bool autoRange: true
        onClicked: {
            if (autoRange) {
                text = "Use Automatic Range";
                scatterGraph.axisX.min = 0.3;
                scatterGraph.axisX.max = 0.7;
                scatterGraph.axisY.min = 0.3;
                scatterGraph.axisY.max = 0.7;
                scatterGraph.axisZ.min = 0.3;
                scatterGraph.axisZ.max = 0.7;
                autoRange = false;
                dragSpeedModifier = 200.0;
            } else {
                text = "Use Preset Range";
                autoRange = true;
                dragSpeedModifier = 100.0;
            }
            scatterGraph.axisX.autoAdjustRange = autoRange;
            scatterGraph.axisY.autoAdjustRange = autoRange;
            scatterGraph.axisZ.autoAdjustRange = autoRange;
        }
    }

    //! [8]
    Button {
        id: orthoToggle
        width: axisDragView.portraitMode ? parent.width : parent.width / 3
        text: "Display Orthographic"
        anchors.left: axisDragView.portraitMode ? parent.left : rangeToggle.right
        anchors.top: axisDragView.portraitMode ? rangeToggle.bottom : parent.top
        onClicked: {
            if (scatterGraph.orthoProjection) {
                text = "Display Orthographic";
                scatterGraph.orthoProjection = false;
                // Orthographic projection disables shadows, so we need to switch them back on
                scatterGraph.shadowQuality = Graphs3D.ShadowQuality.Medium
            } else {
                text = "Display Perspective";
                scatterGraph.orthoProjection = true;
            }
        }
    }
    //! [8]

    Button {
        id: exitButton
        width: axisDragView.portraitMode ? parent.width : parent.width / 3
        text: "Quit"
        anchors.left: axisDragView.portraitMode ? parent.left : orthoToggle.right
        anchors.top: axisDragView.portraitMode ? orthoToggle.bottom : parent.top
        onClicked: Qt.quit();
    }
}
