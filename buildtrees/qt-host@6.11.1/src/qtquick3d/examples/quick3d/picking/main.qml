// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick3D

Window {
    visible: true
    width: 800
    height: 500
    title: qsTr("Picking Example")
    color: "#848895"

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 8
        spacing: 10
        Column {
            Label {
                color: "white"
                font.pointSize: 14
                text: "Last Pick:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Screen Position:"
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

            Label {
                color: "white"
                font.pointSize: 14
                text: "World Normal:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Local Normal:"
            }
        }
        Column {
            Label {
                id: pickName
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: pickPosition
                color: "white"
                font.pointSize: 14
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
            Label {
                id: worldNormal
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: localNormal
                color: "white"
                font.pointSize: 14
            }

        }
    }

    View3D {
        id: view
        anchors.fill: parent
        renderMode: View3D.Underlay

        PointLight {
            x: -200
            y: 200
            z: 300
            quadraticFade: 0
            brightness: 1.5
        }

        PerspectiveCamera {
            z: 500
        }

        //! [pickable model]
        Model {
            id: cubeModel
            objectName: "Cube"
            source: "#Cube"
            pickable: true
            property bool isPicked: false
            //! [pickable model]

            scale.x: 1.5
            scale.y: 2
            scale.z: 1.5

            //! [picked color]
            materials: PrincipledMaterial {
                baseColor: cubeModel.isPicked ? "#41cd52" : "#09102b"
                //! [picked color]
                roughnessMap: Texture { source: "maps/roughness.jpg" }
                roughness: 1
            }

            //! [picked animation]
            SequentialAnimation on eulerRotation {
                running: !cubeModel.isPicked
                //! [picked animation]
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 2500
                    from: Qt.vector3d(0, 0, 0)
                    to: Qt.vector3d(360, 360, 360)
                }
            }
        }

        Model {
            id: coneModel
            objectName: "Cone"
            source: "#Cone"
            pickable: true
            property bool isPicked: false

            x: 200
            z: 100

            scale.x: 2
            scale.y: 1.5
            scale.z: 2

            materials: PrincipledMaterial {
                baseColor: coneModel.isPicked ? "#21be2b" : "#53586b"
                roughnessMap: Texture { source: "maps/roughness.jpg" }
                roughness: 1
            }

            SequentialAnimation on eulerRotation {
                running: !coneModel.isPicked
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 10000
                    from: Qt.vector3d(0, 0, 0)
                    to: Qt.vector3d(-360, 360, 0)
                }
            }
        }

        Model {
            id: sphereModel
            objectName: "Sphere"
            source: "#Sphere"
            pickable: true
            property bool isPicked: false

            x: -100
            y: -100
            z: -100

            scale.x: 5
            scale.y: 3
            scale.z: 1

            materials: PrincipledMaterial {
                baseColor: sphereModel.isPicked ? "#17a81a" : "#9d9faa"
                roughnessMap: Texture { source: "maps/roughness.jpg" }
                roughness: 1
            }

            SequentialAnimation on eulerRotation.x {
                running: !sphereModel.isPicked
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 5000
                    from: 0
                    to: 360
                }
            }
        }
    }

    //! [mouse area]
    MouseArea {
        anchors.fill: view
        //! [mouse area]

        onClicked: (mouse) => {
            // Get screen coordinates of the click
            pickPosition.text = "(" + mouse.x + ", " + mouse.y + ")"
            //! [pick result]
            var result = view.pick(mouse.x, mouse.y);
            //! [pick result]
            //! [pick specifics]
            if (result.objectHit) {
                var pickedObject = result.objectHit;
                // Toggle the isPicked property for the model
                pickedObject.isPicked = !pickedObject.isPicked;
                // Get picked model name
                pickName.text = pickedObject.objectName;
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
                worldNormal.text = "("
                        + result.sceneNormal.x.toFixed(2) + ", "
                        + result.sceneNormal.y.toFixed(2) + ", "
                        + result.sceneNormal.z.toFixed(2) + ")";
                localNormal.text = "("
                        + result.normal.x.toFixed(2) + ", "
                        + result.normal.y.toFixed(2) + ", "
                        + result.normal.z.toFixed(2) + ")";
                //! [pick specifics]
            } else {
                pickName.text = "None";
                uvPosition.text = "";
                distance.text = "";
                scenePosition.text = "";
                localPosition.text = "";
                worldNormal.text = "";
                localNormal.text = "";
            }
        }
    }
}
