// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Rectangle {
    width: 640
    height: 480
    color: "lightgray"

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "white"
            backgroundMode: SceneEnvironment.SkyBox
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            lightProbe: proceduralSky
        }

        Texture {
            id: proceduralSky
            textureData: ProceduralSkyTextureData {
                sunLongitude: -115
            }
        }

        Node {
            id: scene
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 360)
                eulerRotation: Qt.vector3d(0, 0, 0)
                clipFar: 1000
                clipNear: 0.1
            }

            DirectionalLight {
                eulerRotation: Qt.vector3d(-45, 25, 0)
            }

            PrincipledMaterial {
                id: glassMaterial
                baseColor: "#aaaacc"
                transmissionFactor: 0.95
                thicknessFactor: 1
                roughness: 0.05
            }

            Model {
                source: "#Sphere"
                materials: glassMaterial
            }
        }
    }
}
