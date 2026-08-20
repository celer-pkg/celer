// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 320
    height: 480
    color: "blue"

    View3D {
        // no width and height set, the result should be an empty blue background,
        // but it should not crash

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
        }
        renderMode: View3D.Offscreen

        Node {
            id: sceneRoot
            PerspectiveCamera {
                id: camera1
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
        camera: camera1
    }
}
