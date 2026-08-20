// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Lancelot

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 3)
            clipNear: 1
            clipFar: 100
        }

        DirectionalLight {
        }

        Model {
            geometry: PointsTopology { }
            materials: DefaultMaterial {
                    diffuseColor: "red"
            }
        }

        Model {
            position: Qt.vector3d(0.5, 0.5, 0.0);
            geometry: PointsTopology { }
            materials: DefaultMaterial {
                pointSize: 5
                diffuseColor: "blue"
            }
        }

        Model {
            position: Qt.vector3d(-0.5, -0.5, 0.0);
            geometry: PointsTopology { }
            materials: CustomMaterial {
                vertexShader: "pointstopology.vert"
                fragmentShader: "pointstopology.frag"
            }
        }
    }
}
