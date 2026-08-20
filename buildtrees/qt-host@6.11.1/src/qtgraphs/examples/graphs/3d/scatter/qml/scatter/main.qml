// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs
//! [0]

Item {
    id: mainView
    width: 800
    height: 600

    property int margin: 30
    property int spacing: 10
    property int radius: 6
    property int buttonMinWidth: 175

    //! [1]
    Data {
        id: seriesData
    }
    //! [1]

    //! [2]
    GraphsTheme {
        id: themeQt
        theme: GraphsTheme.Theme.QtGreen
        labelFont.pointSize: 40
    }

    GraphsTheme {
        id: themeQtNeonGreen
        theme: GraphsTheme.Theme.QtGreenNeon
        colorScheme: GraphsTheme.ColorScheme.Dark
    }
    //! [2]

    //! [5]
    component CustomButton : RoundButton {
        id: buttonRoot
        //! [5]
        //! [6]
        property alias source: iconImage.source

        Layout.minimumWidth: buttonMinWidth
        Layout.fillWidth: true

        radius: mainView.radius

        background: Rectangle {
            radius: mainView.radius
            color: "white"
            border.color: "black"
        }
        //! [6]
        //! [7]
        contentItem: Row {
            id: content
            Image {
                id: iconImage
            }
            Label {
                text: buttonRoot.text
                horizontalAlignment: Text.AlignLeft
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        //! [7]
    }

    //! [3]
    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: margin
        spacing: spacing
        //! [3]
        //! [4]
        GridLayout {
            Layout.fillWidth: true
            rowSpacing: spacing
            columnSpacing: spacing

            columns: mainView.width < mainView.buttonMinWidth * 2 + mainView.spacing + mainView.margin * 2 // width of 2 buttons
                     ? 1
                     : (mainView.width < mainView.buttonMinWidth * 4 + mainView.spacing * 3 + mainView.margin * 2 // width of 4 buttons
                        ? 2
                        : 4)
            //! [4]
            //! [8]
            CustomButton {
                id: shadowButton
                text: graph.shadowQuality === Graphs3D.ShadowQuality.None ?
                          qsTr("Show Shadows") : qsTr("Hide Shadows")
                source: graph.shadowQuality === Graphs3D.ShadowQuality.None ?
                            "qrc:/images/shadow.svg" : "qrc:/images/shadow_hide.svg"
                onClicked: {
                    graph.shadowQuality = graph.shadowQuality === Graphs3D.ShadowQuality.None ?
                                Graphs3D.ShadowQuality.High :
                                Graphs3D.ShadowQuality.None
                }
            }
            //! [8]

            CustomButton {
                id: smoothButton
                text: qsTr("Smooth Series One")
                source: graph.meshSmooth ?
                            "qrc:/images/flatten.svg" : "qrc:/images/smooth_curve.svg"
                onClicked: {
                    text = graph.meshSmooth ? qsTr("Smooth Series One") : qsTr("Flat Series One")
                    graph.meshSmooth = !graph.meshSmooth
                }
            }

            CustomButton {
                id: cameraButton
                text: qsTr("Camera Placement")
                source: graph.cameraPreset === Graphs3D.CameraPreset.Front ?
                            "qrc:/images/camera.svg" : "qrc:/images/camera2.svg"
                onClicked: {
                    graph.cameraPreset = graph.cameraPreset === Graphs3D.CameraPreset.Front ?
                                Graphs3D.CameraPreset.IsometricRightHigh :
                                Graphs3D.CameraPreset.Front
                }
            }

            CustomButton {
                id: backgroundButton
                text: qsTr("Hide Background")
                source: graph.theme.backgroundVisible ?
                            "qrc:/images/background_hide.svg" : "qrc:/images/background.svg"
                onClicked: {
                    graph.theme.plotAreaBackgroundVisible = !graph.theme.plotAreaBackgroundVisible
                    text = graph.theme.plotAreaBackgroundVisible ? qsTr("Hide Graph Background") : qsTr("Show Graph Background")
                }
            }
        }

        //! [9]
        Graph {
            id: graph
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        //! [9]

        //! [10]
        CustomButton {
            id: themeButton
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: false
            //! [10]
            text: qsTr("Change Theme")
            source: "qrc:/images/theme.svg"
            onClicked: {
                graph.theme = graph.theme.theme === GraphsTheme.Theme.QtGreenNeon ? themeQt : themeQtNeonGreen
                backgroundButton.text = graph.theme.plotAreaBackgroundVisible ? qsTr("Hide Graph Background") : qsTr("Show Graph Background")
            }
        }
    }
}
