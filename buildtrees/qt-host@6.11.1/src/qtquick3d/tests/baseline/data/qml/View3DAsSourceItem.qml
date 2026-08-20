// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 600
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
        }

        View3D {
            id: textureSourceView3D
            width: 256
            height: 256
            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Color
                clearColor: "green"
            }
            DirectionalLight { }
            PerspectiveCamera { z: 600 }
            Model {
                eulerRotation: Qt.vector3d(30, 30, 0)
                source: "#Cube"
                materials: PrincipledMaterial { }
            }
        }

        Model {
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    sourceItem: textureSourceView3D
                }
            }
        }
    }
}
