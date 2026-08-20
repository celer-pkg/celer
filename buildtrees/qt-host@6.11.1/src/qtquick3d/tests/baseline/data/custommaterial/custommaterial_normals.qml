// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

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
            lightProbe: Texture {
                source: "../shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
            }
            probeExposure: 1
        }

        PerspectiveCamera {
            id: camera
            position.z: 0.5
            position.y: 1.75
            eulerRotation.x: -62
            clipNear: 1
            clipFar: 100
        }

        DirectionalLight {

        }

        Model {
            source: "../shared/models/plane.mesh"
            eulerRotation.x: -75

            materials: [
                CustomMaterial {
                    vertexShader: "custommaterial_normals.vert"
                    fragmentShader: "custommaterial_normals.frag"
                    property TextureInput heightMap: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/heightmap.png"
                        }
                    }
                    property TextureInput normalMap: TextureInput {
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/wrinkles_normal.jpg"
                        }
                    }
                }
            ]
        }
    }
}
