// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Model {
    id: beamModel
    pickable: false
    property color color: "red"

    function generate(beamParts, segments, tubeRadius) {
        beamMesh.generateMesh(beamParts,  segments, tubeRadius)
    }

    function show() {
        opacity = 0.7
    }

    function hide() {
        opacity = 0.
    }

    visible: opacity > 0.1
    opacity: 0.7
    Behavior on opacity {
        NumberAnimation {
            duration: 100
        }
    }

    materials: [
        PrincipledMaterial {
            baseColor: beamModel.color
            lighting: PrincipledMaterial.NoLighting
        }
    ]

    geometry: ProceduralMesh {
        id: beamMesh
        function generateMesh(beamParts, segments = 3, tubeRadius = 1) {

            let verts = []
            let norms = []
            let uvs = []
            let indices = []

            const rings = beamParts.length

            for (var i = 0; i < rings; ++i) {
                const pos = beamParts[i]
                const fwd = i > 0 ? beamParts[i].minus(beamParts[i - 1]) : beamParts[1].minus(beamParts[0])
                const quat = Quaternion.fromAxisAndAngle(fwd, 360/segments)
                let perp = Qt.vector3d(0, tubeRadius, 0)
                for (var j = 0; j <= segments; ++j) {
                    const posX = pos.x + perp.x
                    const posY = pos.y + perp.y
                    const posZ = pos.z + perp.z
                    perp = quat.times(perp)

                    verts.push(Qt.vector3d(posX, posY, posZ));

                    const normal = Qt.vector3d(posX - pos.x, posY, posZ - pos.z).normalized();
                    norms.push(normal);

                    uvs.push(Qt.vector2d(i / rings, j / segments));
                }
            }

            for (let i = 0; i < rings - 1; ++i) {
                for (let j = 0; j < segments; ++j) {
                    const a = (segments + 1) * i + j;
                    const b = (segments + 1) * (i + 1) + j;
                    const c = (segments + 1) * (i + 1) + j + 1;
                    const d = (segments + 1) * i + j + 1;

                    // Generate two triangles for each quad in the mesh
                    // Adjust order to be counter-clockwise
                    indices.push(a, d, b);
                    indices.push(b, d, c);
                }
            }

            positions = verts
            normals = norms
            uv0s = uvs
            indexes = indices
        }
    }
}
