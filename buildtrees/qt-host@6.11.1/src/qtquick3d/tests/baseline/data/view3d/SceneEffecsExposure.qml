// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: rootItem
    width: 460
    height: 460

    Node {
        id: scene
        PerspectiveCamera {
            id: camera
            z: 100
        }

        DirectionalLight {

        }

        Model {
            source: "#Sphere"
            materials: [ PrincipledMaterial {
                    id: sphereMaterial
                    baseColor: "green"
                    metalness: 0.5
                } ]
        }
    }

    GridLayout {
        columns: 2
        anchors.fill: parent

        View3D {
            implicitHeight: rootItem.height / 2
            implicitWidth: rootItem.width / 2
            environment: ExtendedSceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBox
                lightProbe: Texture {
                    source: "../shared/maps/TestEnvironment-512.hdr"
                    mappingMode: Texture.LightProbe
                }

                probeExposure: 0
                exposure: 1.0
            }

            importScene: scene
        }

        View3D {
            implicitHeight: rootItem.height / 2
            implicitWidth: rootItem.width / 2
            environment: ExtendedSceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBox
                lightProbe: Texture {
                    source: "../shared/maps/TestEnvironment-512.hdr"
                    mappingMode: Texture.LightProbe
                }

                probeExposure: 10.0
                exposure: 1.0
            }

            importScene: scene
        }

        View3D {
            implicitHeight: rootItem.height / 2
            implicitWidth: rootItem.width / 2
            environment: ExtendedSceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBox
                lightProbe: Texture {
                    source: "../shared/maps/TestEnvironment-512.hdr"
                    mappingMode: Texture.LightProbe
                }

                probeExposure: 1.0
                exposure: 1.0
            }

            importScene: scene
        }

        View3D {
            implicitHeight: rootItem.height / 2
            implicitWidth: rootItem.width / 2
            environment: ExtendedSceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBox
                lightProbe: Texture {
                    source: "../shared/maps/TestEnvironment-512.hdr"
                    mappingMode: Texture.LightProbe
                }

                probeExposure: 1.0
                exposure: 10.0
            }

            importScene: scene
        }
    }
}
