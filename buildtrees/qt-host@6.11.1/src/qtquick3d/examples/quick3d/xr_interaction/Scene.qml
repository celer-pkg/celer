// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Node {
    InstanceRepeater {
        instancingTable: RandomInstancing {
            instanceCount: 61 // '@'..'~'
            position: InstanceRange {
                from: Qt.vector3d(-50, 75, -50)
                to: Qt.vector3d(150, 175, 50)
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 360, 360)
            }
            gridSpacing: Qt.vector3d(8, 8, 8)
        }
        Model {
            geometry: ExtrudedTextGeometry {
                text: String.fromCharCode(64 + index)
                scale: 25
                depth: 3
                font.pointSize: 72
            }
            materials: PrincipledMaterial {
                baseColor: Qt.hsva(Math.random(), 0.5, 1.0, 1.0)
                roughness: 0.2
                metalness: 1
            }
            pickable: true
        }
    }

    Model {
        source: "#Sphere"
        materials: PrincipledMaterial {
            baseColor: "orange"
            metalness: 1.0
            roughness: 0.1
        }
        position: Qt.vector3d(0, 125, -75)
        scale: "0.1, 0.2, 0.3"
        pickable: true
    }

    Model {
        id: floor
        source: "#Rectangle"
        eulerRotation.x: -90
        scale: Qt.vector3d(5,5,5)
        materials: [ PrincipledMaterial {
                baseColor: "green"
            }
        ]
    }
}
