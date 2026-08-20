// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls

Window {
    visible: true
    width: 1280
    height: 720
    title: qsTr("Subset Picking Manual Test")

    View3D {
        id: viewport
        anchors.fill: parent

        property list<Model> modelSubset

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
            instanceCount: 100

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
            id: instanceRepeater
            instancingTable: randomWithData
            Model {
                id: delegate
                source: "#Cube"
                pickable: true
                property int instanceIndex: index // used for indexLabel.text
                property color instanceColor: modelColor
                property bool isBlue: false
                property bool isYellow: false
                materials: PrincipledMaterial {
                    baseColor: isYellow ? Qt.color("yellow") : isBlue ? Qt.color("blue") : instanceColor
                }

                Component.onCompleted: {
                    // A small chance it will be part of the subset

                    if (Math.random() > 0.9) {
                        instanceColor = Qt.color("green")
                        viewport.modelSubset.push(this)
                    } else {
                        instanceColor = Qt.color("red")
                    }
                }
            }
        }

    } // View3D

    Timer {
        interval: 5000; running: true; repeat: true; triggeredOnStart: true
        onTriggered: {
            // Reset yellow and blue flag and set new random blue ones and one yellow

            for (var i = 0; i < viewport.modelSubset.length; ++i) {
                viewport.modelSubset[i].isBlue = false;
                viewport.modelSubset[i].isYellow = false;
            }

            // Up to 5 blue ones, may be less if same one is picked multiple times
            for (var j = 0; j < 5; ++j) {
                var index = Math.floor(viewport.modelSubset.length * Math.random());
                viewport.modelSubset[index].isBlue = true;
            }

            index = Math.floor(viewport.modelSubset.length * Math.random());
            viewport.modelSubset[index].isBlue = false; // Just to make sure it's not blue and yellow
            viewport.modelSubset[index].isYellow = true;
        }
    }

    MouseArea {
        anchors.fill: viewport

        onClicked: (mouse) => {
            // Get screen coordinates of the click
            positionLabel.text = "(" + mouse.x + ", " + mouse.y + ")"

            // Total subset hits (green + blue + yellow)
            var resultList = viewport.pickSubset(mouse.x, mouse.y, viewport.modelSubset);
            subsetHits.text = resultList.length;

            // Create dynamic array of blue ones,
            // which is again a subset of the subset
            var blueArray = []
            for (var i = 0; i < viewport.modelSubset.length; ++i) {
               if (viewport.modelSubset[i].isBlue)
                   blueArray.push(viewport.modelSubset[i])
            }

            // Total blue hits
            resultList = viewport.pickSubset(mouse.x, mouse.y, blueArray);
            blueHits.text = resultList.length;

            // Find the yellow one and pick that one
            for (var j = 0; j < viewport.modelSubset.length; ++j) {
                if (viewport.modelSubset[j].isYellow) {
                    var result = viewport.pick(mouse.x, mouse.y, viewport.modelSubset[j]);
                    yellowHit.text = result.objectHit ? "true" : "false";
                    break;
                }
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
                text: "Number of subset hits:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Number of blue hits:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Hit yellow:"
            }
        }
        Column {
            Label {
                id: positionLabel
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: subsetHits
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: blueHits
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: yellowHit
                color: "white"
                font.pointSize: 14
            }
        }
    }
}
