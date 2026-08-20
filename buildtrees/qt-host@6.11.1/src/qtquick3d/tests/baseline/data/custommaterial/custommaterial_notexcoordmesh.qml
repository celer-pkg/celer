// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: "black"

    View3D {
        anchors.fill: parent
        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
        }

        DirectionalLight {
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        Model {
            scale: Qt.vector3d(40, 40, 40)
            // use a mesh that does not have attr_uv0
            // (the teapot here is good for this test because it only has attr_pos and attr_norm)
            source: "../shared/models/teapot_without_texcoords.mesh"
            x: -100
            materials: [
                CustomMaterial {
                    TextureInput {
                        id: texInput
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                        }
                    }

                    vertexShader: "custommaterial_notexcoordmesh.vert"
                    fragmentShader: "custommaterial_notexcoordmesh.frag"
                    property real selector: 0.0
                    property TextureInput tex: texInput
                }
            ]
        }

        Model {
            scale: Qt.vector3d(40, 40, 40)
            source: "../shared/models/teapot_without_texcoords.mesh"
            x: 100
            materials: [
                CustomMaterial {
                    vertexShader: "custommaterial_notexcoordmesh.vert"
                    fragmentShader: "custommaterial_notexcoordmesh.frag"
                    property real selector: 1.0
                    property TextureInput tex: texInput
                }
            ]
        }
    }
}
