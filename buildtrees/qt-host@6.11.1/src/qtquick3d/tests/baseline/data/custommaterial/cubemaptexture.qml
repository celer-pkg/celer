// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 600
    height: 600
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
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        CubeMapTexture {
            id: cubemapTexture
            source: "../shared/maps/cubemap_bc1.ktx"
        }

        CustomMaterial {
            id: cubeMapMaterial
            property TextureInput cubemap: TextureInput { texture: cubemapTexture }
            fragmentShader: "cubemaptexture.frag"
        }

        // Using the same ktx (with 6 faces) with Texture should work and give
        // us just face 0 (X+).
        Texture {
            id: cubemapSourceAsNonCubemapTexture
            source: "../shared/maps/cubemap_bc1.ktx"
        }

        CustomMaterial {
            id: notReallyCubemapMaterial
            property TextureInput notReallyCubemap: TextureInput { texture: cubemapSourceAsNonCubemapTexture }
            fragmentShader: "cubemap_as_2d.frag"
        }

        Model {
            source: "#Sphere"
            x: -100
            scale: Qt.vector3d(4, 4, 4)
            materials: cubeMapMaterial
        }

        Model {
            source: "#Sphere"
            x: 220
            scale: Qt.vector3d(2, 2, 2)
            materials: notReallyCubemapMaterial
        }
    }
}
