// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

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
            position.z: 5
            clipNear: 1
            clipFar: 100
        }

        DirectionalLight {

        }

        Model {
            source: "../shared/models/cube_uv2.mesh"
            position: Qt.vector3d(-1, -1, 0);
            eulerRotation.x: 45
            eulerRotation.y: 30
            materials: [
                CustomMaterial {
                    vertexShader: "custommaterial_uvs.vert"
                    fragmentShader: "custommaterial_uvs.frag"
                    property vector2d offset0: Qt.vector2d(0, 0)
                    property vector2d offset1: Qt.vector2d(0, 0)
                    property TextureInput tex1: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                        }
                    }
                    property TextureInput tex2: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/rgba.png"
                        }
                    }
                }
            ]
        }

        Model {
            source: "../shared/models/cube_uv2.mesh"
            position: Qt.vector3d(1, 1, 0);
            eulerRotation.x: 45
            eulerRotation.y: 30
            materials: [
                CustomMaterial {
                    vertexShader: "custommaterial_uvs.vert"
                    fragmentShader: "custommaterial_uvs.frag"
                    property vector2d offset0: Qt.vector2d(0.5, 0)
                    property vector2d offset1: Qt.vector2d(0.5, 0.5)
                    property TextureInput tex1: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                        }
                    }
                    property TextureInput tex2: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/rgba.png"
                        }
                    }
                }
            ]
        }
    }

}
