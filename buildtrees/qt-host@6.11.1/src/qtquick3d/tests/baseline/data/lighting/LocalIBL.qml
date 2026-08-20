// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 400
    height: 400
    color: "black"

    View3D {
        anchors.fill: parent

        PerspectiveCamera {
            id: cam
            position: Qt.vector3d(50, 50, 150)
        }

        DirectionalLight {
            rotation: Quaternion.fromEulerAngles(-30, -70, 0)
            ambientColor: Qt.rgba(0.8, 0.8, 0.8, 1.0);
        }

        environment: SceneEnvironment {
            probeExposure: 7.5
        }

        Texture {
            id: lightprobe_texture
            source: "maps/OpenfootageNET_Gerlos-512.hdr"
            mappingMode: Texture.LightProbe
            tilingModeHorizontal: Texture.ClampToEdge
        }

        Model {
            id: no_ibl
            position: Qt.vector3d(0, 100, -100)
            source: "#Sphere"
            materials: [
                PrincipledMaterial {
                    baseColor: "#ffd777"
                    metalness: 0.7
                    roughness: 0.3
                    specularAmount: 0.2
                    specularTint: 0.0
                    opacity: 1.0
                    lighting: DefaultMaterial.FragmentLighting
                }
            ]
        }

        Model {
            id: local_lightprobe
            position: Qt.vector3d(100, 100, -100)
            source: "#Sphere"
            materials: [
                PrincipledMaterial {
                    baseColor: "#e7e7f7"
                    metalness: 1.0
                    roughness: 0.3
                    specularAmount: 0.2
                    specularTint: 0.0
                    opacity: 1.0
                    lighting: DefaultMaterial.FragmentLighting

                    lightProbe: lightprobe_texture
                }
            ]
        }

        Model {
            id: specular_reflection_map
            position: Qt.vector3d(100, 0, -100)
            source: "#Sphere"
            materials: [
                DefaultMaterial {
                    diffuseColor: "red"
                    lighting: DefaultMaterial.FragmentLighting

                    specularReflectionMap: lightprobe_texture
                }
            ]
        }
    }
}
