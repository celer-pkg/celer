// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {
    id: stand

    Firewood {
        instancing: FileInstancing {
            source: "media/meshes/firewood_instances.xml.bin"
        }
    }

    Repeater3D {
        model: 4
        delegate: StaticRigidBody {
            id: standDelegate
            required property int index
            property list<vector3d> poses : [
                Qt.vector3d(1000, 0, 1000),
                Qt.vector3d(1000, 0, -1000),
                Qt.vector3d(-1000, 0, 1000),
                Qt.vector3d(-1000, 0, -1000)
            ]
            scale: Qt.vector3d(3, 1.9, 3)
            position: Qt.vector3d(standDelegate.poses[index].x,
                                  50 * scale.y,
                                  standDelegate.poses[index].z)
            collisionShapes: BoxShape {}

            Model {
                source: "#Cube"
                pickable: true
                materials: [InvisibleMaterial]
            }
        }
    }

    // Resources
    Texture {
        id: block_bc
        generateMipmaps: true
        source: "media/textures/block/block_bc.jpg"
    }
    Texture {
        id: block_mr
        generateMipmaps: true
        source: "media/textures/block/block_mr.jpg"
    }
    Texture {
        id: block_n
        generateMipmaps: true
        source: "media/textures/block/block_n.jpg"
    }
    PrincipledMaterial {
        id: block_material
        baseColorMap: block_bc
        metalnessMap: block_mr
        metalnessChannel: PrincipledMaterial.R
        roughnessMap: block_mr
        roughnessChannel: PrincipledMaterial.G
        metalness: 1
        roughness: 1
        normalMap: block_n
    }

    Model {
        y: 50
        scale: Qt.vector3d(50, 50, 50)
        instancing: FileInstancing {
            source: "media/meshes/block_instances.xml.bin"
        }

        source: "media/meshes/block/defaultMaterial_mesh.mesh"
        materials: [
            block_material
        ]
    }
}
