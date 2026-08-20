// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 320
    height: 480
    color: "blue"

    Image {
        source: "../shared/maps/checkerboard_1.png"
        anchors.fill: parent
    }


    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
        }
        renderMode: View3D.Offscreen

        Node {
            id: sceneRoot
            PerspectiveCamera {
                id: camera2

                x: -300
                z: 300
                rotation: Quaternion.fromEulerAngles(0, -45, 0)
            }

            DirectionalLight {

            }

            Model {
                source: "#Cube"
                materials: DefaultMaterial {
                    diffuseColor: "red"
                }
            }
        }
        camera: camera2
    }

    Image {
        source: "../shared/maps/checkerboard_2.png"
        anchors.fill: parent
    }
}
