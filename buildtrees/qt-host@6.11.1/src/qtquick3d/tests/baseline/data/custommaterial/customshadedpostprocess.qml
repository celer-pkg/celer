// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: root
    width: 600
    height: 400
    color: "black"

    View3D {
        anchors.fill: parent
        camera: camera

        PerspectiveCamera {
            id: camera
            z: 20
        }

        DirectionalLight {
            id: light
            eulerRotation.x: 20
        }

        Model {
            x: -10
            y: -5
            scale: Qt.vector3d(3.0, 3.0, 3.0)
            source: "../shared/models/teapot_without_texcoords.mesh"
            materials: [
                PrincipledMaterial {
                    metalness: 0.5
                    baseColor: "#ffff0000"
                }
            ]
        }

        Model {
            y: -5
            scale: Qt.vector3d(3.0, 3.0, 3.0)
            source: "../shared/models/teapot_without_texcoords.mesh"
            materials: [
                CustomMaterial {
                    fragmentShader: "customshadedpostprocess.frag"
                    property real screen_width: root.width
                }
            ]
        }

        Model {
            x: 10
            y: -5
            scale: Qt.vector3d(3.0, 3.0, 3.0)
            source: "../shared/models/teapot_without_texcoords.mesh"
            materials: [
                PrincipledMaterial {
                    metalness: 0.5
                    emissiveFactor: Qt.vector3d(0.0, 1.0, 0.0)
                }
            ]
        }
    }
}
