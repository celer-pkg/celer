// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Node {
    id: root
    property SceneEnvironment qmlxr_environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.SkyBoxCubeMap
        clearColor: "pink" // Just to verify that the skybox covers everything
        lightProbe: skyTexture
        skyBoxCubeMap: skyboxTexture
    }

    CubeMapTexture {
        id: skyboxTexture
        source: "maps/skybox/right.jpg;maps/skybox/left.jpg;maps/skybox/top.jpg;maps/skybox/bottom.jpg;maps/skybox/front.jpg;maps/skybox/back.jpg"
    }

    Texture {
        id: skyTexture
        textureData: ProceduralSkyTextureData {
            id: proceduralSkyTextureData
            groundBottomColor: "#775533"
            groundHorizonColor: "green"
            groundCurve: 0.11

            skyTopColor: "#ddeeff"
            skyHorizonColor: "#aaaaff"
            skyCurve: 0.15
        }
    }


    DirectionalLight {
    }

    Model {
        source: "#Rectangle"
        scale: "5, 5, 5"
        materials: PrincipledMaterial { baseColor: "darkgray" }
        eulerRotation.x: -90
        y: -150
    }


    Repeater3D {
        model: 4
        Node {
            eulerRotation.y: 45 + 90 * index
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "#ffcc77"
                    metalness: 1
                    roughness: 0.3
                }

                z: 250
                scale: Qt.vector3d(0.5, 0.5, 0.5)


                NumberAnimation  on eulerRotation.y {
                    duration: 10000
                    from: 0
                    to: 360
                    running: true
                    loops: -1
                }
            }
        }
    }

    Node {
        z: -200
        Text {
            text: "Qt 6 in VR"
            font.pointSize: 12
            color: "white"
            x: - width / 2 // align center
        }
    }
}
