// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 320
    height: 480
    color: "blue"


    View3D {
        anchors.fill: parent

        Node {
            id: sceneRoot
            PerspectiveCamera {
                id: camera1
                z: -600
            }
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
                materials: DefaultMaterial {}
            }
        }
        camera: camera2
    }
}
