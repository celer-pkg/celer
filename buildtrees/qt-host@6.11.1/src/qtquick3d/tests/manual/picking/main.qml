// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls

Window {
    visible: true
    width: 1280
    height: 720
    title: qsTr("Picking Manual Test")

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "gray"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            PerspectiveCamera {
                z: 1000
            }

            PropertyAnimation on eulerRotation.y {
                from: 0
                to: 360
                running: true
                duration: 100000
                loops: -1
            }
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            ambientColor: "#999"
            brightness: 1
        }


        RandomInstancing {
            id: randomWithData
            instanceCount: 1000

            position: InstanceRange {
                from: Qt.vector3d(-500, -400, -500)
                to: Qt.vector3d(500, 400, 500)
            }
            scale: InstanceRange {
                from: Qt.vector3d(0.1, 0.1, 0.1)
                to: Qt.vector3d(0.7, 0.7, 0.7)
                proportional: true
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 360, 360)
            }
            colorModel: RandomInstancing.HSV
            color: InstanceRange {
                from: Qt.hsva(0, 0.1, 0.6, 1)
                to: Qt.hsva(1, 0.3, 0.8, 1)
            }
        }

        // Just using the instance table as a way to create a lot of random items quickly

        InstanceRepeater {
            instancingTable: randomWithData
            Model {
                id: delegate
                source: "#Cube"
                pickable: true
                property int instanceIndex: index // used for indexLabel.text
                property color instanceColor: modelColor
                property bool picked: false
                materials: PrincipledMaterial {
                    baseColor: picked ? Qt.hsva(modelColor.hsvHue, 1, 1) : modelColor
                }
            }
        }



    } // View3D


    MouseArea {
        anchors.fill: viewport

        //! [mouse area]

        onClicked: (mouse) => {
            // Get screen coordinates of the click
            positionLabel.text = "(" + mouse.x + ", " + mouse.y + ")"
            //! [pick result]
            var result = viewport.pick(mouse.x, mouse.y);
            //! [pick result]
            //! [pick specifics]
            if (result.objectHit) {
                var pickedObject = result.objectHit;
                // Get other pick specifics
                uvPosition.text = "("
                        + result.uvPosition.x.toFixed(2) + ", "
                        + result.uvPosition.y.toFixed(2) + ")";
                distance.text = result.distance.toFixed(2);
                scenePosition.text = "("
                        + result.scenePosition.x.toFixed(2) + ", "
                        + result.scenePosition.y.toFixed(2) + ", "
                        + result.scenePosition.z.toFixed(2) + ")";
                localPosition.text = "("
                        + result.position.x.toFixed(2) + ", "
                        + result.position.y.toFixed(2) + ", "
                        + result.position.z.toFixed(2) + ")";
                colorRect.color =  pickedObject.instanceColor
                pickedObject.picked = true
                //! [pick specifics]
            } else {
                uvPosition.text = "";
                distance.text = "";
                scenePosition.text = "";
                localPosition.text = "";
                colorRect.color = "transparent"
            }
        }
    }

    Row {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8
        spacing: 10
        Column {
            Label {
                color: "white"
                font.pointSize: 14
                text: "Screen Position:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Color:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "UV Position:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Distance:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "World Position:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Local Position:"
            }

        }
        Column {
            Label {
                id: positionLabel
                color: "white"
                font.pointSize: 14
            }
            Rectangle {
                id: colorRect
                height: positionLabel.height
                width: 100
                color: "transparent"
            }
            Label {
                id: uvPosition
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: distance
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: scenePosition
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: localPosition
                color: "white"
                font.pointSize: 14
            }
        }
    }
}
