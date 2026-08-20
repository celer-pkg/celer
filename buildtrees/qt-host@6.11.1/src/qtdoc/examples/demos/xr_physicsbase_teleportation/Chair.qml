// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {
    id: node0

    // Resources
    Texture {
        id: chair_bc
        generateMipmaps: true
        source: "media/textures/chair/chair_bc.jpg"
    }
    Texture {
        id: chair_mr
        generateMipmaps: true
        source: "media/textures/chair/chair_mr.jpg"
    }
    Texture {
        id: chair_n
        generateMipmaps: true
        source: "media/textures/chair/chair_n.jpg"
    }
    PrincipledMaterial {
        id: chair_material
        baseColorMap: chair_bc
        metalnessMap: chair_mr
        metalnessChannel: PrincipledMaterial.R
        roughnessMap: chair_mr
        roughnessChannel: PrincipledMaterial.G
        metalness: 1
        roughness: 1
        normalMap: chair_n
        occlusionMap: chair_mr
        occlusionChannel: PrincipledMaterial.B
    }

    // Nodes:
    DynamicRigidBody {
        collisionShapes: BoxShape {
            extents: Qt.vector3d(90, 200, 90)
        }
        Model {
            y: -100
            pickable: true
            scale: Qt.vector3d(1.7, 1., 1.7)
            source: "#Cone"
            materials: [InvisibleMaterial]
        }
        Node {
            scale: Qt.vector3d(100, 100, 100)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            Node {
                rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
                Node {
                    id: node3_low3
                    objectName: "3_low"
                    Model {
                        source: "media/meshes/chair/defaultMaterial_mesh5.mesh"
                        materials: [
                            chair_material
                        ]
                    }
                }
                Node {
                    Model {
                        source: "media/meshes/chair/defaultMaterial_mesh13.mesh"
                        materials: [
                            chair_material
                        ]
                    }
                }
                Node {
                    Model {
                        source: "media/meshes/chair/defaultMaterial_mesh16.mesh"
                        materials: [
                            chair_material
                        ]
                    }
                }
                Node {
                    Model {
                        source: "media/meshes/chair/defaultMaterial_mesh19.mesh"
                        materials: [
                            chair_material
                        ]
                    }
                }
            }
        }
    }

    // Animations:
}
