// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 900
    height: 450
    color: "lightgray"

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(300, -135, 300)
        }

        SpotLight {
            y: -100
            x: 330
            z: 600
            brightness: 10
        }

        Repeater3D {
            id: xRepeater
            model: 11

            Repeater3D {
                id: yRepeater
                model: 11
                property int xValue: index
                x: xValue * 60
                scale: Qt.vector3d(10, 10, 10)
                Model {
                    property int yValue: index
                    y: yValue * -3
                    source: "../shared/models/teapot_without_texcoords.mesh"
                    materials: PrincipledMaterial {
                        baseColor: Qt.rgba(1, 0, 0, 1)
                        metalness: 1.0
                        roughness: yValue / 10.0
                        specularAmount: xValue / 10.0
                        specularTint: 0.0
                    }
                }
            }
        }
    }
}
