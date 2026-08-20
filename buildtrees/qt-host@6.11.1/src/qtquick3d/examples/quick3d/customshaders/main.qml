// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    color: "#848895"

    MaterialControl {
        id: control
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    View3D {
        anchors.fill: parent
        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
        }

        //! [use]
        Model {
            position: Qt.vector3d(0, 0, 0)
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 3000
                loops: -1
                running: control.animateRotation
            }
            scale: Qt.vector3d(2, 2, 2)
            source: "#Sphere"
            materials: [
                ExampleMaterial {
                    id: exampleMaterial
                    time: control.time
                    amplitude: control.amplitude
                    alpha: control.alpha
                    texturing: control.texturing
                    textureFromItem: control.textureFromItem
                    texSrc: Rectangle {
                        layer.enabled: true
                        layer.textureMirroring: ShaderEffectSource.NoMirroring
                        visible: false
                        SequentialAnimation on color {
                            ColorAnimation { from: "black"; to: "yellow"; duration: 2000 }
                            ColorAnimation { from: "yellow"; to: "cyan"; duration: 1000 }
                            ColorAnimation { from: "cyan"; to: "black"; duration: 500 }
                            loops: -1
                        }
                        width: 512
                        height: 512
                        Image {
                            source: "qt_logo.png"
                            anchors.centerIn: parent
                        }
                    }
                }
            ]
        }
        //! [use]
    }
}
