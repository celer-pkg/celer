// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 600
    height: 600
    color: Qt.rgba(0, 0, 0, 1)

    GridView {
        id: grid
        anchors.fill: parent
        cellWidth: 300
        cellHeight: 300
        model: 4

        delegate: View3D {
            width: grid.cellWidth
            height: grid.cellHeight

            environment: SceneEnvironment {
                clearColor: "#444845"
                backgroundMode: SceneEnvironment.Color
            }

            camera: camera

            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 600)
                eulerRotation.x: index == 0 ? 0 : index == 1 ? -20 : index == 2 ? -165 : -250
                eulerRotation.y: index == 0 ? 0 : index == 1 ? 35 : index == 2 ? 150 : 190
            }

            DirectionalLight {
                position: Qt.vector3d(-500, 500, -100)
                ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
            }

            CubeMapTexture {
                id: skyboxTexture
                source: "../shared/maps/fishpond_bc1.ktx"
            }

            Model {
                source: "#Cube"
                position: camera.position
                scale: Qt.vector3d(20, 20, 20)
                materials: CustomMaterial {
                    property TextureInput skybox: TextureInput { texture: skyboxTexture }
                    cullMode: Material.FrontFaceCulling
                    shadingMode: CustomMaterial.Unshaded
                    vertexShader: "basicskybox.vert"
                    fragmentShader: "basicskybox.frag"
                }
            }
        }
    }
}
