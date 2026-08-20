// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma Singleton

import QtQuick
import QtQuick3D
import QtMultimedia
import QtQuick3D.Helpers

Node {
    property alias fireVideo: video_out
    property alias fireMesh: fire_mesh
    VideoOutput {
        id: video_out
        fillMode: VideoOutput.Stretch
        width: 512
        height: 512
        visible: false
        MediaPlayer {
            id: player
            source: "media/textures/fire.mp4"
            loops: MediaPlayer.Infinite
            autoPlay: true
            videoOutput: FireResources.fireVideo
        }
    }

    ProceduralMesh {
        id: fire_mesh
        Component.onCompleted: {
            generateMesh()
        }
        function generateMesh() {

            let verts = []
            let uvs = []
            let indices = []
            let colrs = []

            const xOffset = 0.
            const zOffset = 0.
            const countX = 5.
            const countZ = 5.
            const segments = 30.
            const height = 100.
            const thickness = 100.
            const thicknessHalf = thickness * 0.5
            const segmentHeight = height / segments

            let pos = Qt.vector3d(0, 0, 0)
            let index = 0
            let blade = 0.0;

            for (let xx = 0.; xx < countX; ++xx) {
                for (let zz = 0.; zz < countZ; ++zz) {

                    pos.x = xx * xOffset
                    pos.z = zz * zOffset

                    const xxNorm = xx / countX
                    const zzNorm = zz / countZ

                    const fBlade = blade / (countX*countZ)
                    ++blade

                    for (let ss = 0.; ss <= segments; ++ss) {

                        const uvY = ss / segments

                        const posLX = pos.x - thicknessHalf
                        const posLY = segmentHeight * ss + pos.y
                        const posLZ = pos.z

                        verts.push(Qt.vector3d(posLX, posLY, posLZ));
                        uvs.push(Qt.vector2d(0, uvY))
                        colrs.push(Qt.vector4d(fBlade, 0, xxNorm, zzNorm))

                        const posRX = pos.x + thicknessHalf
                        const posRY = segmentHeight * ss + pos.y
                        const posRZ = pos.z

                        verts.push(Qt.vector3d(posRX, posRY, posRZ));
                        uvs.push(Qt.vector2d(1.0, uvY))
                        colrs.push(Qt.vector4d(fBlade, 0, xxNorm, zzNorm))

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


