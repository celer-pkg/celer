// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Model {
    id: floor
    property int aoState: 0
    receivesReflections: true
    materials: [floorMat_local]

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: floorMat_local
            fragmentShader: "shaders/floor.frag"
            vertexShader: "shaders/floor.vert"
            cullMode: Material.BackFaceCulling
            property color baseColor: "#ffffff"
            property color subsurfaceColor: "#ffffff"
            property real specular: 0.75
            property real density: 0.5
            property real opacity: 1
            property real roughness: 0
            property real clearcoat: 0.25
            property real metalness: 0.0
            property real sssDistortion: 0.0
            property real sssPower: 10
            property real sssScale: 2
            property vector3d aoMaskScale: Qt.vector3d(1, 1, 1)
            property vector3d aoMaskPos: Qt.vector3d(0, 0, 1)
            shadingMode: CustomMaterial.Shaded
            objectName: "New Material"
            property color aoMask: "#ff0000"
            property TextureInput aoTexture: aoTexInput
            property TextureInput paramsTexture: paramsTexInput
            property TextureInput normalsTexture: normalTexInput
            alwaysDirty: false
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            sourceBlend: CustomMaterial.NoBlend
            TextureInput {
                id: aoTexInput
                texture: aoTex
                enabled: true
            }

            Texture {
                id: aoTex
                source: "file:content/images/AOBake.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
            }

            TextureInput {
                id: normalTexInput
                texture: normalTex
                enabled: true
            }

            Texture {
                id: normalTex
                source: "file:content/images/Asphalt010_2K_NormalGL.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
            }

            TextureInput {
                id: paramsTexInput
                texture: paramsTex
                enabled: true
            }

            Texture {
                id: paramsTex
                source: "file:content/images/Asphalt010_2K_Roughness.png"
                scaleV: 3
                scaleU: 3
                mipFilter: Texture.Linear
                generateMipmaps: true
            }
        }
    }
    states: [
        State {
            name: "Dragon"
            when: floor.aoState === 1

            PropertyChanges {
                target: floorMat_local
                aoMask: "#00ff00"
                aoMaskScale: Qt.vector3d(0.7, 0.7, 1.0)
                aoMaskPos: Qt.vector3d(0, 0, 0)
            }
        },
        State {
            name: "Bunny"
            when: floor.aoState === 2

            PropertyChanges {
                target: floorMat_local
                aoMaskScale: Qt.vector3d(0.9, 0.9, 1.0)
                aoMaskPos: Qt.vector3d(0.01, 0.01, 0.0)
                aoMask: "#0000ff"
            }
        },
        State {
            name: "matball"
            when: floor.aoState === 0

            PropertyChanges {
                target: floorMat_local
                aoMaskPos: Qt.vector3d(0, 0, 0.0)
                aoMaskScale: Qt.vector3d(1, 1, 1.0)
                aoMask: "#ff0000"
            }
        },
        State {
            name: "none"
            when: floor.aoState === 3

            PropertyChanges {
                target: floorMat_local
                aoMaskPos: Qt.vector3d(0, 0, 0.0)
                aoMaskScale: Qt.vector3d(1, 1, 1.0)
                aoMask: "#000000"
            }
        }
    ]
}
