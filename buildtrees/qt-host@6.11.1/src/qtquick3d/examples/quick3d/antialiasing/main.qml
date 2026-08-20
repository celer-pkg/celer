// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Quick3D Antialiasing Example")

    property bool isLandscape: width > height

    View3D {
        id: view3D
        property real animationValue: 0.0
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: window.isLandscape ? settingsPane.right : parent.left
        anchors.top: window.isLandscape ? parent.top : settingsPane.bottom

        SequentialAnimation {
            id: modelAnimation
            running: false
            NumberAnimation {
                target: view3D
                property: "animationValue"
                from: 0.0
                to: 1.0
                duration: 1000
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: view3D
                property: "animationValue"
                from: 1.0
                to: 0.0
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }

        PerspectiveCamera {
            z: 500
        }

        DirectionalLight {
            eulerRotation.x: -30
        }
   //! [scene environment]
        environment: SceneEnvironment {
            id: sceneEnvironment
            clearColor: "#002b36"
            backgroundMode: SceneEnvironment.Color

            antialiasingMode: modeButton1.checked ? SceneEnvironment.NoAA : modeButton2.checked
                                                    ? SceneEnvironment.SSAA : modeButton3.checked
                                                      ? SceneEnvironment.MSAA : SceneEnvironment.ProgressiveAA

            antialiasingQuality: qualityButton1.checked ? SceneEnvironment.Medium : qualityButton2.checked
                                                          ? SceneEnvironment.High : SceneEnvironment.VeryHigh
            temporalAAEnabled: temporalModeButton.checked
            temporalAAStrength: temporalStrengthSlider.value
            temporalAAMode: temporalModeDefault.checked ? SceneEnvironment.TAADefault : SceneEnvironment.TAAMotionVector
        }
   //! [scene environment]

        Node {
            id: scene
            x: -80

            Model {
                source: "#Cube"
                eulerRotation.y: 45
                eulerRotation.x: 30 + view3D.animationValue * 100
                scale: Qt.vector3d(2, 2, 2)
                materials: PrincipledMaterial {
                    baseColor: "#4aee45"
                }
            }

            Model {
                source: "#Cube"
                x: 200
                y: 150 + view3D.animationValue * 10
                eulerRotation.y: 5
                eulerRotation.x: 5
                scale: Qt.vector3d(0.5, 0.5, 0.5)
                materials: PrincipledMaterial {
                    baseColor: "#faee45"
                }
            }

            Model {
                source: "#Sphere"
                x: 120
                y: -40
                z: 160 + view3D.animationValue * 40
                scale: Qt.vector3d(1.5, 1.5, 1.5)
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.8, 0.8, 0.8, 1.0)
                }
            }
        }
    }


    Pane {
        id: settingsPane
        width: window.isLandscape ? implicitWidth : window.width
        height: window.isLandscape ? window.height : window.height * 0.33
        ScrollView {
            anchors.fill: parent
            ColumnLayout {
                id: settingsArea
                GroupBox {
                    title: qsTr("Antialiasing Mode")
                    ColumnLayout {
                        RadioButton {
                            id: modeButton1
                            checked: true
                            text: qsTr("NoAA")
                        }
                        RadioButton {
                            id: modeButton2
                            text: qsTr("SSAA")
                        }
                        RadioButton {
                            id: modeButton3
                            text: qsTr("MSAA")
                        }
                        RadioButton {
                            id: modeButton4
                            text: qsTr("ProgressiveAA")
                        }
                    }
                }

                GroupBox {
                    title: qsTr("Antialiasing Quality")
                    enabled: !modeButton1.checked
                    ButtonGroup {
                        buttons: antialiasingQualityColumn.children
                    }
                    ColumnLayout {
                        id: antialiasingQualityColumn
                        RadioButton {
                            id: qualityButton1
                            text: qsTr("Medium")
                        }
                        RadioButton {
                            id: qualityButton2
                            checked: true
                            text: qsTr("High")
                        }
                        RadioButton {
                            id: qualityButton3
                            text: qsTr("VeryHigh")
                        }
                    }
                }

                CheckBox {
                    id: temporalModeButton
                    text: qsTr("Enable Temporal AA")
                }

                GroupBox {
                    title: qsTr("Temporal AA Mode")
                    enabled: temporalModeButton.checked
                    ButtonGroup {
                        buttons: temporalModeColumn.children
                    }
                    ColumnLayout {
                        id: temporalModeColumn
                        RadioButton {
                            id: temporalModeDefault
                            checked: true
                            text: qsTr("Default")
                        }
                        RadioButton {
                            id: temporalModeMotionVector
                            text: qsTr("Motion Vector")
                        }
                    }
                }

                ColumnLayout {
                    enabled: temporalModeButton.checked
                    Label {
                        text: qsTr("Temporal AA Strength")
                    }

                    RowLayout {
                        Slider {
                            id: temporalStrengthSlider
                            from: 0.0
                            to: 2.0
                            value: 0.3
                        }
                        Label {
                            text: temporalStrengthSlider.value.toFixed(1);
                        }
                    }
                }

                Button {
                    id: animationButton
                    Layout.alignment: Qt.AlignHCenter
                    text: "Animate!"
                    onClicked: {
                        modelAnimation.restart();
                    }
                }
            }
        }
    }

    Pane {
        anchors.top: view3D.top
        anchors.right: parent.right
        Label {
            id: debugViewToggleText
            text: dbg.visible ? "Hide DebugView" : "Show DebugView"
            anchors.right: parent.right
            anchors.top: parent.top
            MouseArea {
                anchors.fill: parent
                onClicked: dbg.visible = !dbg.visible
                DebugView {
                    y: debugViewToggleText.height * 2
                    anchors.right: parent.right
                    source: view3D
                    id: dbg
                    visible: false
                }
            }
        }
    }
}
