// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

ProceduralMesh {
        property real partitions: 50
        property real segments: 1
        property real radius: 50.0
        property real height: 50.0
        property real width: 80
        property var meshArrays: generateMesh(partitions , width, height, radius)
        positions: meshArrays.verts
        normals: meshArrays.normals
        uv0s: meshArrays.uvs
        indexes: meshArrays.indices

        function generateMesh(partitions : real, width: real, height: real, radius: real) : var {
            let verts = []
            let normals = []
            let uvs = []
            let indices = []

            // width = angleSpan * radius
            const angleSpan = width / radius

            for (let i = 0; i <= partitions ; ++i) {
                for (let j = 0; j <= 1; ++j) {
                    const u = i / partitions ;
                    const v = j;

                    const x = u - 0.5 // centered
                    const angle = x * angleSpan

                    const posZ = radius - radius * Math.cos(angle)
                    const posY = height * v
                    const posX = radius * Math.sin(angle)

                    verts.push(Qt.vector3d(posX, posY, posZ));

                    let normal = Qt.vector3d(0 - posX, 0, radius - posZ).normalized();
                    normals.push(normal);

                    uvs.push(Qt.vector2d(u, v));
                }
            }

            for (let i = 0; i < partitions ; ++i) {
                let a = (segments + 1) * i;
                let b = (segments + 1) * (i + 1);
                let c = (segments + 1) * (i + 1) + 1;
                let d = (segments + 1) * i + 1;

                // Generate two triangles for each quad in the mesh
                // Adjust order to be counter-clockwise
                indices.push(a, b, d);
                indices.push(b, c, d);
            }
            return { verts: verts, normals: normals, uvs: uvs, indices: indices }
        }
    }
