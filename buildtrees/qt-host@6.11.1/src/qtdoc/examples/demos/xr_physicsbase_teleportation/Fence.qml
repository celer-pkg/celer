// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {

    StaticRigidBody {
        objectName: "Fence"
        scale: Qt.vector3d(1990, 1990, 1990)
        eulerRotation: Qt.vector3d(-90, 0, 0)
        collisionShapes: TriangleMeshShape {
            source: "media/meshes/cylinder_invert.mesh"
        }

        Model {
            source: "media/meshes/cylinder_invert.mesh"
            pickable: true
            materials: [
                InvisibleMaterial
            ]
        }
    }

    Model {
        id: model
        source: "#Cylinder"
        scale: Qt.vector3d(0.3, 3., 0.3)
        position: Qt.vector3d(0, 50 * scale.y, 0)

        instancing: FileInstancing {
            source: "media/meshes/fence_instances.xml.bin"
        }

        materials: [
            PrincipledMaterial {
                baseColorMap: Texture {
                    generateMipmaps: true
                    source: "media/textures/bark_bc.jpg"
                    scaleU: 3
                    scaleV: 8
                }
                roughness: 1.
                normalMap: Texture {
                    generateMipmaps: true
                    source: "media/textures/bark_n.jpg"
                    scaleU: 3
                    scaleV: 8
                }
            }
        ]
    }
}
