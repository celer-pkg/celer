// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {
    id: node1

    // Resources
    Texture {
        id: table_bc
        generateMipmaps: true
        source: "media/textures/table/table_bc.jpg"
    }
    Texture {
        id: table_mr
        generateMipmaps: true
        source: "media/textures/table/table_mr.jpg"
    }
    Texture {
        id: table_n
        generateMipmaps: true
        source: "media/textures/table/table_n.jpg"
    }
    PrincipledMaterial {
        id: table_material
        baseColorMap: table_bc
        metalnessMap: table_mr
        metalnessChannel: PrincipledMaterial.R
        roughnessMap: table_mr
        roughnessChannel: PrincipledMaterial.G
        metalness: 1
        roughness: 1
        normalMap: table_n
        occlusionMap: table_mr
        occlusionChannel: PrincipledMaterial.B
    }

    // Nodes:
    DynamicRigidBody {
        collisionShapes: BoxShape {
            y: -2
            extents: Qt.vector3d(122, 103, 200)
        }
        Model {
            pickable: true
            scale: Qt.vector3d(1.22, 1.05, 2.0)
            source: "#Cube"
            materials: [InvisibleMaterial]
        }

        Node {
            scale: Qt.vector3d(100, 100, 100)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            Node {
                rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
                Node {
                    id: rail_low19
                    objectName: "Metal_low"
                    Model {
                        source: "media/meshes/table/defaultMaterial_mesh7.mesh"
                        materials: [
                            table_material
                        ]
                    }
                }
                Node {
                    id: legs_low10
                    objectName: "Legs_low"
                    Model {
                        source: "media/meshes/table/defaultMaterial_mesh17.mesh"
                        materials: [
                            table_material
                        ]
                    }
                }
                Node {
                    id: corners_low14
                    objectName: "Corners_low"
                    Model {
                        source: "media/meshes/table/defaultMaterial_mesh20.mesh"
                        materials: [
                            table_material
                        ]
                    }
                }
                Node {
                    id: base_low17
                    objectName: "Base_low"
                    Model {
                        source: "media/meshes/table/defaultMaterial_mesh23.mesh"
                        materials: [
                            table_material
                        ]
                    }
                }
                Node {
                    id: defaultMaterial15
                    objectName: "Rail_low"
                    Model {
                        source: "media/meshes/table/defaultMaterial_mesh26.mesh"
                        materials: [
                            table_material
                        ]
                    }
                }
                Node {
                    id: table_low23
                    objectName: "Table_low"
                    Model {
                        source: "media/meshes/table/defaultMaterial_mesh27.mesh"
                        materials: [
                            table_material
                        ]
                    }
                }
            }
        }
    }

    // Animations:
}
