// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
import QtQuick
import QtQuick.Controls
import QtDataVisualization
//! [0]

//! [1]
Item {
    id: mainView
    //! [1]
    width: 1600
    height: 1200

    property bool portraitMode: width < height
    // Adjust the button width based on screen orientation:
    // If we're in portrait mode, just fit two buttons side-by-side, otherwise
    // fit all of the buttons side-by-side.
    property real buttonWidth: mainView.portraitMode ? (mainView.width / 2 - 8)
                                                     : (mainView.width / 6 - 6)

    //! [4]
    Data {
        id: seriesData
    }
    //! [4]

    //! [13]
    Theme3D {
        id: themeQt
        type: Theme3D.ThemeQt
        font.pointSize: 40
    }
    //! [13]

    Theme3D {
        id: themeRetro
        type: Theme3D.ThemeRetro
    }

    //! [8]
    //! [9]
    Item {
        id: dataView
        anchors.bottom: parent.bottom
        //! [9]
        width: parent.width
        // Adjust the space based on screen orientation:
        // If we're in portrait mode, we have 3 rows of buttons, otherwise they are all in one row.
        height: parent.height - (mainView.portraitMode ? shadowToggle.implicitHeight * 3 + 25
                                                       : shadowToggle.implicitHeight + 10)
        //! [8]

        //! [2]
        Scatter3D {
            id: scatterGraph
            anchors.fill: parent
            //! [2]
            //! [3]
            theme: themeQt
            shadowQuality: AbstractGraph3D.ShadowQualityHigh
            scene.activeCamera.cameraPreset: Camera3D.CameraPresetFront
            //! [3]
            //! [6]
            axisX.segmentCount: 3
            axisX.subSegmentCount: 2
            axisX.labelFormat: "%.2f"
            axisZ.segmentCount: 2
            axisZ.subSegmentCount: 2
            axisZ.labelFormat: "%.2f"
            axisY.segmentCount: 2
            axisY.subSegmentCount: 2
            axisY.labelFormat: "%.2f"
            //! [6]
            //! [5]
            Scatter3DSeries {
                id: scatterSeries
                //! [5]
                //! [10]
                itemLabelFormat: "Series 1: X:@xLabel Y:@yLabel Z:@zLabel"
                //! [10]

                //! [11]
                ItemModelScatterDataProxy {
                    itemModel: seriesData.model
                    xPosRole: "xPos"
                    yPosRole: "yPos"
                    zPosRole: "zPos"
                }
                //! [11]
            }

            //! [12]
            Scatter3DSeries {
                id: scatterSeriesTwo
                itemLabelFormat: "Series 2: X:@xLabel Y:@yLabel Z:@zLabel"
                itemSize: 0.05
                mesh: Abstract3DSeries.MeshCube
                //! [12]

                ItemModelScatterDataProxy {
                    itemModel: seriesData.modelTwo
                    xPosRole: "xPos"
                    yPosRole: "yPos"
                    zPosRole: "zPos"
                }
            }
            Scatter3DSeries {
                id: scatterSeriesThree
                itemLabelFormat: "Series 3: X:@xLabel Y:@yLabel Z:@zLabel"
                itemSize: 0.1
                mesh: Abstract3DSeries.MeshMinimal

                ItemModelScatterDataProxy {
                    itemModel: seriesData.modelThree
                    xPosRole: "xPos"
                    yPosRole: "yPos"
                    zPosRole: "zPos"
                }
            }
        }
    }

    //! [7]
    Button {
        id: shadowToggle
        width: mainView.buttonWidth // Calculated elsewhere based on screen orientation
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 5
        text: scatterGraph.shadowsSupported ? "Hide Shadows" : "Shadows not supported"
        enabled: scatterGraph.shadowsSupported
        onClicked: {
            if (scatterGraph.shadowQuality === AbstractGraph3D.ShadowQualityNone) {
                scatterGraph.shadowQuality = AbstractGraph3D.ShadowQualityHigh;
                text = "Hide Shadows";
            } else {
                scatterGraph.shadowQuality = AbstractGraph3D.ShadowQualityNone;
                text = "Show Shadows";
            }
        }
    }
    //! [7]

    Button {
        id: smoothToggle
        width: mainView.buttonWidth
        anchors.left: shadowToggle.right
        anchors.top: parent.top
        anchors.margins: 5
        text: "Use Smooth for Series One"
        onClicked: {
            if (!scatterSeries.meshSmooth) {
                text = "Use Flat for Series One";
                scatterSeries.meshSmooth = true;
            } else {
                text = "Use Smooth for Series One";
                scatterSeries.meshSmooth = false;
            }
        }
    }

    Button {
        id: cameraToggle
        width: mainView.buttonWidth
        anchors.left: mainView.portraitMode ? parent.left : smoothToggle.right
        anchors.top: mainView.portraitMode ? smoothToggle.bottom : parent.top
        anchors.margins: 5
        text: "Change Camera Placement"
        onClicked: {
            if (scatterGraph.scene.activeCamera.cameraPreset === Camera3D.CameraPresetFront) {
                scatterGraph.scene.activeCamera.cameraPreset =
                        Camera3D.CameraPresetIsometricRightHigh;
            } else {
                scatterGraph.scene.activeCamera.cameraPreset = Camera3D.CameraPresetFront;
            }
        }
    }

    Button {
        id: themeToggle
        width: mainView.buttonWidth
        anchors.left: cameraToggle.right
        anchors.top: mainView.portraitMode ? smoothToggle.bottom : parent.top
        anchors.margins: 5
        text: "Change Theme"
        onClicked: {
            if (scatterGraph.theme.type === Theme3D.ThemeRetro)
                scatterGraph.theme = themeQt;
            else
                scatterGraph.theme = themeRetro;
            if (scatterGraph.theme.backgroundEnabled)
                backgroundToggle.text = "Hide Background";
            else
                backgroundToggle.text = "Show Background";
        }
    }

    Button {
        id: backgroundToggle
        width: mainView.buttonWidth
        anchors.left: mainView.portraitMode ? parent.left : themeToggle.right
        anchors.top: mainView.portraitMode ? themeToggle.bottom : parent.top
        anchors.margins: 5
        text: "Hide Background"
        onClicked: {
            if (scatterGraph.theme.backgroundEnabled) {
                scatterGraph.theme.backgroundEnabled = false;
                text = "Show Background";
            } else {
                scatterGraph.theme.backgroundEnabled = true;
                text = "Hide Background";
            }
        }
    }

    Button {
        id: exitButton
        width: mainView.buttonWidth
        anchors.left: backgroundToggle.right
        anchors.top: mainView.portraitMode ? themeToggle.bottom : parent.top
        anchors.margins: 5
        text: "Quit"
        onClicked: Qt.quit();
    }
}
