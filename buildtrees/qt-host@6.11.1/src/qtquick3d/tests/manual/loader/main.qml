// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick3D

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Loader Test")

    Component {
        id: cubeModel
        Model {
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseColor: "red"
            }
            PropertyAnimation on eulerRotation.x {
                from: 0
                to: 360
                running: true
                duration: 1000
                loops: Animation.Infinite
            }
        }
    }

    View3D {
        id: view
        anchors.fill: parent

        DirectionalLight {

        }

        PerspectiveCamera {
            z: 600
        }
        Repeater3D {
            model: 100
            Loader3D {
                id: modelLoader
                position: Qt.vector3d(randomWithRange(-300, 300),
                                      randomWithRange(-300, 300),
                                      randomWithRange(-300, 300))
                source: "TestModel.qml"

                Timer {
                    interval: 1000
                    running: true
                    repeat: true
                    property bool toggle: true
                    onTriggered: {
                        if (toggle)
                            modelLoader.sourceComponent = cubeModel;
                        else
                            modelLoader.source = "TestModel.qml"
                        toggle = !toggle;
                    }
                }
            }
        }
    }

    function randomWithRange(min, max) {
        return Math.floor(Math.random() * (max - min + 1) ) + min;
    }

}
