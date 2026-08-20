// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D

Node {
    id: node

    property alias color: material.baseColor

    PrincipledMaterial {
        id: material
        roughness: 0.4
        clearcoatAmount: 0.5
    }
    Node {
        id: sketchfab_model
        objectName: "Sketchfab_model"
        position: Qt.vector3d(4.98882, 6.45249, 1.31884e-06)
        Node {
            id: handle
            rotation: Qt.quaternion(0.680931, -0.680931, -0.190613, 0.190613)
            scale: Qt.vector3d(1.50044, 1.50044, 1.50044)
            Model {
                source: "meshes/handle.mesh"
                materials: [
                    material
                ]
            }
            Node {
                id: cup
                position: Qt.vector3d(-5.85351, 5.49194e-07, -1.47589)
                rotation: Qt.quaternion(0.962981, -5.65672e-24, 0.269568, -3.36567e-24)
                scale: Qt.vector3d(66.6473, 66.6473, 66.6473)
                Model {
                    source: "meshes/cup.mesh"
                    materials: [
                        material
                    ]
                }
            }
        }
    }
}
