// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtGraphs
import "."

Item {
    id: mainView
    width: 1280
    height: 720

    property real maxSegmentCount: 10
    property real minSegmentCount: 1
    property var activeGraph: scatterGraph
    property var cameraAnimationX: scatterCameraAnimationX
    property var cameraAnimationY: scatterCameraAnimationY

    Data {
        id: graphData
    }

    Item {
        id: dataView
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height - buttonLayout.height

        Surface3D {
            id: surfaceGraph
            width: dataView.width
            height: dataView.height
            visible: activeGraph == surfaceGraph
            theme: GraphsTheme { theme: GraphsTheme.Theme.QtGreen }
            shadowQuality: Graphs3D.ShadowQuality.Medium
            cameraYRotation: 30.0

            Surface3DSeries {
                id: surfaceSeries
                itemLabelFormat: "One - X:@xLabel Y:@yLabel Z:@zLabel"
                mesh: Abstract3DSeries.Mesh.Cube


                ItemModelSurfaceDataProxy {
                    itemModel : ListModel {
                        id: surfaceDataModel
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

                        ListElement{ coords: "0,0"; data: "40.0/30.0/14.75"; }
                        ListElement{ coords: "1,0"; data: "41.1/30.3/13.00"; }
                        ListElement{ coords: "2,0"; data: "42.5/30.7/11.24"; }
                        ListElement{ coords: "3,0"; data: "44.0/30.5/12.53"; }
                        ListElement{ coords: "0,1"; data: "40.2/31.2/13.55"; }
                        ListElement{ coords: "1,1"; data: "41.3/31.5/13.03"; }
                        ListElement{ coords: "2,1"; data: "42.6/31.7/13.46"; }
                        ListElement{ coords: "3,1"; data: "43.4/31.5/14.12"; }
                        ListElement{ coords: "0,2"; data: "40.2/32.3/13.37"; }
                        ListElement{ coords: "1,2"; data: "41.1/32.4/12.98"; }
                        ListElement{ coords: "2,2"; data: "42.5/32.1/13.33"; }
                        ListElement{ coords: "3,2"; data: "43.3/32.7/13.23"; }
                        ListElement{ coords: "0,3"; data: "40.7/33.3/15.34"; }
                        ListElement{ coords: "1,3"; data: "41.5/33.2/14.54"; }
                        ListElement{ coords: "2,3"; data: "42.4/33.6/14.65"; }
                        ListElement{ coords: "3,3"; data: "43.2/33.4/16.67"; }
                        ListElement{ coords: "0,4"; data: "40.6/35.0/16.01"; }
                        ListElement{ coords: "1,4"; data: "41.3/34.6/15.83"; }
                        ListElement{ coords: "2,4"; data: "42.5/34.8/17.32"; }
                        ListElement{ coords: "3,4"; data: "43.7/34.3/16.90"; }

                    }

                    rowRole: "coords"
                    columnRole: "coords"
                    xPosRole: "data"
                    zPosRole: "data"
                    yPosRole: "data"
                    rowRolePattern: /(\d),\d/
                    columnRolePattern: /(\d),(\d)/
                    xPosRolePattern: /^([asd]*)([fgh]*)([jkl]*)[^\/]*\/([^\/]*)\/.*$/
                    yPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    zPosRolePattern: /^([asd]*)([qwe]*)([tyu]*)([fgj]*)([^\/]*)\/[^\/]*\/.*$/
                    rowRoleReplace: "\\1"
                    columnRoleReplace: "\\2"
                    xPosRoleReplace: "\\4"
                    yPosRoleReplace: "\\3"
                    zPosRoleReplace: "\\5"
                }
             }

            Component.onCompleted: {
                surfaceGraph.unsetDefaultInputHandler();
            }

            onQueriedGraphPositionChanged:
                console.log("Queried Position : " + queriedGraphPosition)
        }

        Scatter3D {
            id: scatterGraph
            visible: activeGraph == scatterGraph
            width: dataView.width
            height: dataView.height
            theme: GraphsTheme { theme: GraphsTheme.Theme.QtGreen }
            shadowQuality: Graphs3D.ShadowQuality.Medium
            cameraYRotation: 30.0

            Scatter3DSeries {
                id: scatterSeriesOne
                itemLabelFormat: "One - X:@xLabel Y:@yLabel Z:@zLabel"
                mesh: Abstract3DSeries.Mesh.Cube

                ItemModelScatterDataProxy {
                    itemModel: graphData.modelOne
                    xPosRole: "xPos"
                    yPosRole: "yPos"
                    zPosRole: "zPos"
                }
            }

            Scatter3DSeries {
                id: scatterSeriesTwo
                itemLabelFormat: "Two - X:@xLabel Y:@yLabel Z:@zLabel"
                mesh: Abstract3DSeries.Mesh.Cube

                ItemModelScatterDataProxy {
                    itemModel: graphData.modelTwo
                    xPosRole: "xPos"
                    yPosRole: "yPos"
                    zPosRole: "zPos"
                }
            }

            Scatter3DSeries {
                id: scatterSeriesThree
                itemLabelFormat: "Three - X:@xLabel Y:@yLabel Z:@zLabel"
                mesh: Abstract3DSeries.Mesh.Cube

                ItemModelScatterDataProxy {
                    itemModel: graphData.modelThree
                    xPosRole: "xPos"
                    yPosRole: "yPos"
                    zPosRole: "zPos"
                }
            }

            Component.onCompleted: {
                scatterGraph.unsetDefaultInputHandler();
            }

            onQueriedGraphPositionChanged:
                console.log("Queried Position : " + queriedGraphPosition)
        }

        MouseArea {
            id: inputArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onWheel: (wheel)=> {
                // Adjust zoom level based on what zoom range we're in.
                var zoomLevel = activeGraph.cameraZoomLevel;
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

                activeGraph.cameraZoomLevel = zoomLevel;
            }
            onClicked: {
                console.log("Queried at: " + Qt.point(mouseX, mouseY))
                activeGraph.scene.graphPositionQuery = Qt.point(mouseX, mouseY)
            }
        }

        Timer {
            id: reselectTimer
            interval: 33
            running: true
            repeat: true
            onTriggered: {
                activeGraph.scene.selectionQueryPosition = Qt.point(-1, -1);
                activeGraph.scene.selectionQueryPosition = Qt.point(inputArea.mouseX, inputArea.mouseY);
            }
        }
    }

    NumberAnimation {
        id: scatterCameraAnimationX
        loops: Animation.Infinite
        running: true
        target: scatterGraph
        property:"cameraXRotation"
        from: 0.0
        to: 360.0
        duration: 20000
    }
    NumberAnimation {
        id: surfaceCameraAnimationX
        loops: Animation.Infinite
        running: true
        target: surfaceGraph
        property:"cameraXRotation"
        from: 0.0
        to: 360.0
        duration: 20000
    }

    SequentialAnimation {
        id: scatterCameraAnimationY
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
    SequentialAnimation {
        id: surfaceCameraAnimationY
        loops: Animation.Infinite
        running: true

        NumberAnimation {
            target: surfaceGraph
            property:"cameraYRotation"
            from: 5.0
            to: 45.0
            duration: 9000
            easing.type: Easing.InOutSine
        }

        NumberAnimation {
            target: surfaceGraph
            property:"cameraYRotation"
            from: 45.0
            to: 5.0
            duration: 9000
            easing.type: Easing.InOutSine
        }
    }

    RowLayout {
        id: buttonLayout

        property int buttonCount: 5

        Layout.minimumHeight: shadowToggle.height
        width: parent.width
        anchors.left: parent.left
        spacing: 0

        Button {
            id: shadowToggle
            Layout.fillHeight: true
            Layout.minimumWidth: parent.width / parent.buttonCount // Buttons divided equally in the layout
            text: "Hide Shadows"
            onClicked: {
                if (activeGraph.shadowQuality === Graphs3D.ShadowQuality.None) {
                    activeGraph.shadowQuality = Graphs3D.ShadowQuality.Medium;
                    text = "Hide Shadows";
                } else {
                    activeGraph.shadowQuality = Graphs3D.ShadowQuality.None;
                    text = "Show Shadows";
                }
            }
        }

        Button {
            id: cameraToggle
            Layout.fillHeight: true
            Layout.minimumWidth: parent.width / parent.buttonCount
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
            id: graphToggle
            Layout.fillHeight: true
            Layout.minimumWidth: parent.width / parent.buttonCount
            text: "Switch graph type"
            onClicked : {
                if (activeGraph == scatterGraph) {
                    activeGraph = surfaceGraph
                    cameraAnimationX = surfaceCameraAnimationX
                    cameraAnimationY = surfaceCameraAnimationY
                    if (surfaceSeries.itemLabelVisible)
                        seriesVisibilityToggle.text = "Hide surface series itemLabel"
                    else
                        seriesVisibilityToggle.text = "Show surface series itemLabel"
                }
                else {
                    activeGraph = scatterGraph
                    cameraAnimationX = scatterCameraAnimationX
                    cameraAnimationY = scatterCameraAnimationY
                    if (scatterSeriesTwo.itemLabelVisible)
                        seriesVisibilityToggle.text = "Hide center series itemLabel"
                    else
                        seriesVisibilityToggle.text = "Show center series itemLabel"
                }
            }
        }

        Button {
            id: seriesVisibilityToggle
            Layout.fillHeight: true
            Layout.minimumWidth: parent.width / parent.buttonCount
            text: "Hide center series itemLabel"
            onClicked : {
                if (activeGraph == scatterGraph) {
                    scatterSeriesTwo.itemLabelVisible = !scatterSeriesTwo.itemLabelVisible

                    if (scatterSeriesTwo.itemLabelVisible)
                        text = "Hide center series itemLabel"
                    else
                        text = "Show center series itemLabel"
                }
                else if (activeGraph == surfaceGraph) {
                    surfaceSeries.itemLabelVisible = !surfaceSeries.itemLabelVisible

                    if (surfaceSeries.itemLabelVisible)
                        text = "Hide surface series itemLabel"
                    else
                        text = "Show surface series itemLabel"
                }
            }
        }

        Button {
            id: exitButton
            Layout.fillHeight: true
            Layout.minimumWidth: parent.width / parent.buttonCount
            text: "Quit"
            onClicked: Qt.quit();
        }
    }

    RowLayout {
        id: sliderLayout
        width: buttonLayout.width / 3
        anchors.top: buttonLayout.bottom
        anchors.left: parent.left
        spacing: 10

        Column {
            id: xColumn
            Label {
                text: "X Axis Segment Sliders"
            }

            Slider {
                id: xSegmentCountSlider
                snapMode: Slider.SnapAlways
                from: mainView.minSegmentCount
                to: mainView.maxSegmentCount
                value: 5

                onValueChanged: activeGraph.axisX.segmentCount = value
            }

            Slider {
                id: xSubSegmentCountSlider
                from: mainView.minSegmentCount
                to: mainView.maxSegmentCount
                value: 1

                onValueChanged: activeGraph.axisX.subSegmentCount = value
            }
        }

        Column {
            id: yColumn
            Label {
                text: "Y Axis Segment Sliders"
            }

            Slider {
                id: ySegmentCountSlider
                clip: true
                from: mainView.minSegmentCount
                to: mainView.maxSegmentCount
                value: 5

                onValueChanged: activeGraph.axisY.segmentCount = value
            }

            Slider {
                id: ySubSegmentCountSlider
                from: mainView.minSegmentCount
                to: mainView.maxSegmentCount
                value: 1

                onValueChanged: activeGraph.axisY.subSegmentCount = value
            }
        }

        Column {
            id: zColumn
            Label {
                text: "Z Axis Segment Sliders"
            }

            Slider {
                id: zSegmentCountSlider
                from: mainView.minSegmentCount
                to: mainView.maxSegmentCount
                value: 5

                onValueChanged: activeGraph.axisZ.segmentCount = value
            }

            Slider {
                id: zSubSegmentCountSlider
                from: mainView.minSegmentCount
                to: mainView.maxSegmentCount
                value: 1

                onValueChanged: activeGraph.axisZ.subSegmentCount = value
            }
        }
    }
}
