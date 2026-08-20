// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


import QtQuick
import QtQuick.Window
import QtQuick3D
import QtQuick3D.Helpers

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("3D repeater")

    function randomWithRange(min, max) {
        return Math.floor(Math.random() * (max - min + 1) ) + min;
    }

    Component {
        id: animatedCube

        Model {
            source: "#Cube"
            position: Qt.vector3d(randomWithRange(-300, 300),
                                  randomWithRange(-300, 300),
                                  randomWithRange(-300, 300))

            property real magnify: 5.0/randomWithRange(10, 100)/randomWithRange(1,10);


            scale: Qt.vector3d(magnify, magnify, magnify)

            materials: DefaultMaterial {
                diffuseColor: Qt.rgba(randomWithRange(0, 255) / 255,
                                      randomWithRange(0, 255) / 255,
                                      randomWithRange(0, 255) / 255,
                                      1.0);
            }
        }
    }

    View3D {
        id: viewport
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
        }

        DirectionalLight {
            id: directionalLight
        }

        Node {

            PerspectiveCamera {
                z: 700
            }

            PropertyAnimation on eulerRotation.y {
                from: 0
                to: 360
                running: true
                duration: 50000
                loops: -1
            }
        }

        Repeater3D {
            eulerRotation: Qt.vector3d(15, 30, 45)
            position: Qt.vector3d(-100, -100, -100)
            model: 8000
            delegate: animatedCube
        }

    }
    DebugView {
        anchors.top: parent.top
        anchors.left: parent.left
        source: viewport
    }

}
