// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 1024
    height: 768
    visible: true
    title: qsTr("Screen Space Reflections")

    View3D {
        id: screenSpaceReflectionsView
        anchors.fill: parent
        property double modelRotation: 0
        property double modelHeight: 0

        NumberAnimation {
            target: screenSpaceReflectionsView
            property: "modelRotation"
            running: true
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }

        SequentialAnimation {
            running: true
            loops: Animation.Infinite
            NumberAnimation {
                target: screenSpaceReflectionsView
                property: "modelHeight"
                from: -5
                to: 20
                duration: 1000
            }
            NumberAnimation {
                target: screenSpaceReflectionsView
                property: "modelHeight"
                from: 20
                to: -5
                duration: 1000
            }
        }

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            probeExposure: 2
            lightProbe: Texture {
                source: "maps/OpenfootageNET_lowerAustria01-1024.hdr"
            }
        }

        //! [scene]
        Node {

            Model {
                source: "#Cube"
                eulerRotation.y: 0
                scale: Qt.vector3d(1, 1, 1)
                position: Qt.vector3d(50.0, 40.0, 50.0)
                materials:  PrincipledMaterial {
                    baseColorMap: Texture {
                        source: "qt_logo_rect.png"
                    }
                }
            }

            Node{

                Model {
                    source: "#Sphere"
                    position: Qt.vector3d(-400.0, screenSpaceReflectionsView.modelHeight, 0.0)
                    materials: PrincipledMaterial {
                        baseColor: "magenta"
                    }
                }
            }

            Node{
                eulerRotation.y: screenSpaceReflectionsView.modelRotation
                position.y: screenSpaceReflectionsView.modelHeight

                Model {
                    source: "#Sphere"
                    pivot: Qt.vector3d(0, 0.0, 0.0)
                    position: Qt.vector3d(200.0, 0.0, 0.0)
                    materials: PrincipledMaterial {
                        baseColor: "green"
                    }
                }
            }

            Node{
                eulerRotation.y: screenSpaceReflectionsView.modelRotation
                position.y: screenSpaceReflectionsView.modelHeight

                Model {
                    source: "#Sphere"
                    eulerRotation.y: 45
                    position: Qt.vector3d(0.0, 0.0, -200.0)
                    materials: PrincipledMaterial {
                        baseColor: "blue"
                    }
                }
            }

            Node{
                eulerRotation.y: screenSpaceReflectionsView.modelRotation
                position.y: screenSpaceReflectionsView.modelHeight

                Model {
                    source: "#Sphere"
                    position: Qt.vector3d(0.0, 0.0, 200.0)
                    materials: PrincipledMaterial {
                        baseColor: "red"
                    }
                }
            }
            //! [scene]

            //! [reflectingsurface]
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(5, 5, 5)
                eulerRotation.x: -90
                eulerRotation.z: 180
                position: Qt.vector3d(0.0, -50.0, 0.0)
                materials: ScreenSpaceReflections {
                    depthBias: depthBiasSlider.value
                    rayMaxDistance: distanceSlider.value
                    marchSteps: marchSlider.value
                    refinementSteps: refinementStepsSlider.value
                    specular: specularSlider.value
                    materialColor: materialColorCheckBox.checked ? "transparent" : "dimgray"
                }
            }
            //! [reflectingsurface]

            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0.0, 40.0, 500)
            }

            DirectionalLight {
                y: 0
                castsShadow: false
            }
        }

        WasdController {
            id: wasdController
            controlledObject: camera
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                wasdController.forceActiveFocus()
            }
        }
    }

    Pane {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10
        Column {
            id: settingsArea
            spacing: 5

            CheckBox {
                id: materialColorCheckBox
                text: "Transparent Material"
            }

            Row {

                Slider {
                    id: depthBiasSlider
                    from: 0.0
                    to: 5
                    value: 0.79
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Depth Bias: " + depthBiasSlider.value.toFixed(2);
                }
            }

            Row {

                Slider {
                    id: distanceSlider
                    from: 0.0
                    to: 400
                    value: 200
                    stepSize: 1
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Ray Distance: " + distanceSlider.value
                }
            }

            Row {

                Slider {
                    id: marchSlider
                    Layout.alignment: Qt.AlignHCenter
                    from: 0
                    to: 2000
                    value: 300
                    stepSize: 1
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "March Steps: " + marchSlider.value
                }
            }

            Row {
                Slider {
                    id: refinementStepsSlider
                    Layout.alignment: Qt.AlignHCenter
                    from: 0
                    to: 50
                    value: 10
                    stepSize: 1
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Refinement Steps: " + refinementStepsSlider.value
                }
            }

            Row {
                Slider {
                    id: specularSlider
                    Layout.alignment: Qt.AlignHCenter
                    from: 0
                    to: 1
                    value: 1.0
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Specular: " + specularSlider.value.toFixed(2);
                }
            }
        }
    }

    Item {
        width: debugViewToggleText.implicitWidth
        height: debugViewToggleText.implicitHeight
        anchors.right: parent.right
        Label {
            id: debugViewToggleText
            text: "Click here " + (dbg.visible ? "to hide DebugView" : "for DebugView")
            anchors.right: parent.right
            anchors.top: parent.top
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dbg.visible = !dbg.visible
            DebugView {
                y: debugViewToggleText.height * 2
                anchors.right: parent.right
                source: screenSpaceReflectionsView
                id: dbg
                visible: false
            }
        }
    }
}
