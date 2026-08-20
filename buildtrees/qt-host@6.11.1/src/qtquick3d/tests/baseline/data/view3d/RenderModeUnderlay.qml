// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

import "../shared/"

Rectangle {
    width: 320
    height: 480
    color: "transparent"

    View3D {
        anchors.fill: parent

        // This is only here to match the QQuickWindow default clear color
        // (white), and so to provide identical results with Qt 5 and 6. It has
        // no effect in Qt 6 due to the renderMode!
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "white"
        }

        renderMode: View3D.Underlay

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
