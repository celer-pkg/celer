// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {
    StaticRigidBody {
        objectName: "Ground"
        eulerRotation: Qt.vector3d(-90, 0, 0)
        collisionShapes: PlaneShape {}
    }
    Model {
        pickable: true
        source: "#Rectangle"
        scale:Qt.vector3d(1000, 1000, 1000)
        eulerRotation: Qt.vector3d(-90, 0, 0)

        materials: [
            PrincipledMaterial {
                baseColorMap: Texture {
                    scaleU: 200
                    scaleV: 200
                    generateMipmaps: true
                    source: "media/textures/ground_bc.jpg"
                }
                normalMap: Texture {
                    scaleU: 200
                    scaleV: 200
                    generateMipmaps: true
                    source: "media/textures/ground_n.jpg"
                }
                roughnessMap: Texture {
                    scaleU: 200
                    scaleV: 200
                    generateMipmaps: true
                    source: "media/textures/ground_r.jpg"
                }
                roughness: 1
            }
        ]
    }
}
