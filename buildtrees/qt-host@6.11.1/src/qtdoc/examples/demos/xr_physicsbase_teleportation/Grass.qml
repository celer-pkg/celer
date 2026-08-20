// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Model {
    id: grassChunkModel
    pickable: false
    property alias windDir: material.windDir
    property alias windstrength: material.windstrength
    property alias baseColor: material.baseColor

    Component.onCompleted: {
        generate()
    }

    function generate() {
        grassChunkMesh.generateMesh()
    }

    function show() {
        opacity = 1.0
    }

    function hide() {
        opacity = 0.
    }

    visible: opacity > 0.1
    opacity: 1.0
    Behavior on opacity {
        NumberAnimation {
            duration: 100
        }
    }

    materials: [
        CustomMaterial {
            id: material
            cullMode: Material.NoCulling
            vertexShader: "media/shaders/grass.vert"
            fragmentShader: "media/shaders/grass.frag"
            property real time: 0
            property color baseColor: "white"

            FrameAnimation {
                id: frameAnimation
                running: true
                onTriggered: {
                    material.time += 0.1 * frameTime
                }
            }

            property real windstrength: 1.0
            property vector2d windDir: Qt.vector2d(0.5, 0.5);

            property TextureInput grass_base : TextureInput {
                texture: Texture {
                    generateMipmaps: true
                    mipFilter: Texture.Linear
                    source: "media/textures/grass_bc.jpg"
                }
            }
            property TextureInput turbulence : TextureInput {
                texture: CommonResources.turbulenceTexture
            }
            property TextureInput perlin : TextureInput {
                texture: Texture {
                    generateMipmaps: true
                    mipFilter: Texture.Linear
                    source: "media/textures/perlin.jpg"
                }
            }
        }
    ]

    geometry: ProceduralMesh {
        id: grassChunkMesh
        function generateMesh() {
            let verts = []
            let uvs = []
            let indices = []
            let colrs = []

            const xOffset = 10.
            const zOffset = 10.
            const countX = 100.
            const countZ = 100.
            const segments = 9.
            const height = 150.
            const thickness = 5.
            const thicknessHalf = thickness * 0.5
            const segmentHeight = height / segments

            let pos = Qt.vector3d(0, 0, 0)
            let index = 0

            const countXHalf = countX / 2;
            const countZHalf = countZ / 2;

            for (let xx = -countXHalf; xx < countXHalf; ++xx) {
                for (let zz = -countZHalf; zz < countZHalf; ++zz) {

                    pos.x = xx * xOffset
                    pos.z = zz * zOffset

                    const xxNorm = xx / countX + 0.5
                    const zzNorm = zz / countZ + 0.5

                    for (let ss = 0; ss <= segments; ++ss) {
                        const uvY = ss / segments

                        const posLX = pos.x - thicknessHalf
                        const posLY = segmentHeight * ss + pos.y
                        const posLZ = pos.z

                        verts.push(Qt.vector3d(posLX, posLY, posLZ))
                        uvs.push(Qt.vector2d(0, uvY))
                        colrs.push(Qt.vector4d(pos.x, pos.z, xxNorm, zzNorm))

                        const posRX = pos.x + thicknessHalf
                        const posRY = segmentHeight * ss + pos.y
                        const posRZ = pos.z

                        verts.push(Qt.vector3d(posRX, posRY, posRZ))
                        uvs.push(Qt.vector2d(1.0, uvY))
                        colrs.push(Qt.vector4d(pos.x, pos.z, xxNorm, zzNorm))

                        if (ss < segments) {
                            indices.push(index, index + 1, index + 2)
                            indices.push(index + 2, index + 1, index + 3)
                        }
                        index += 2
                    }
                }
            }

            positions = verts
            uv0s = uvs
            colors = colrs
            indexes = indices
        }
    }
}
