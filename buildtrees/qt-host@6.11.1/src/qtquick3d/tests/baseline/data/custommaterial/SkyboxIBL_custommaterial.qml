// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 1024
    height: 600
    color: "black"

    View3D {
        anchors.fill: parent
        camera: camera

        DirectionalLight {
            eulerRotation.y: -100
            brightness: 1
        }

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                source: "../lighting/maps/OpenfootageNET_garage-1024.hdr"
            }
            probeOrientation: Qt.vector3d(0, -90, 0)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        Model {
            position: Qt.vector3d(-250, -30, 0)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            materials: [ CustomMaterial {
                    fragmentShader: "custommaterial_ibl.frag"
                }
            ]
        }

        Model {
            position: Qt.vector3d(250, -30, 0)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            materials: [ CustomMaterial {
                    fragmentShader: "custommaterial_ibl2.frag"

                    property TextureInput baseColorMap: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../lighting/maps/metallic/basecolor.jpg"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                    property TextureInput metalnessMap: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../lighting/maps/metallic/metallic.jpg"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                    property TextureInput roughnessMap: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../lighting/maps/metallic/roughness.jpg"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                    property TextureInput normalMap: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../lighting/maps/metallic/normal.jpg"
                            generateMipmaps: true
                            mipFilter: Texture.Linear
                        }
                    }
                }
            ]
        }
    }
}
