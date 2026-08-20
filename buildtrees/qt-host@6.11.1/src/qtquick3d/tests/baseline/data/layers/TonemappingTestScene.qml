// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    PrincipledMaterial {
        id: default_002
        metalness: 1.0
        roughness: 0.5
        baseColor: "lightblue"
    }

    Model {
        id: sphere
        position: Qt.vector3d(-354.989, 135.238, 0)
        source: "#Sphere"
        materials: [default_002]
    }

    Model {
        id: cone
        position: Qt.vector3d(-365.912, -248.222, 0)
        scale: Qt.vector3d(2.89542, 3.13161, 1)
        source: "#Cone"
        materials: [default_002]
    }

    Model {
        id: cube
        position: Qt.vector3d(349.297, -228.053, 0)
        rotation: Quaternion.fromEulerAngles(28.0299, 33.3145, 17.1637)
        scale: Qt.vector3d(2.00606, 1, 1)
        source: "#Cube"

        PrincipledMaterial {
            id: default_003
            metalness: 0.0
            roughness: 0.1
            baseColor: "green"
            specularAmount: 0.8
        }
        materials: [default_003]
    }

    Node {
        id: barrel
        position: Qt.vector3d(-292.216, -304.023, -434)
        rotation: Quaternion.fromEulerAngles(0, 0, -41.5)
        scale: Qt.vector3d(10, 10, 10)

        Model {
            id: barrel_1
            rotation: Quaternion.fromEulerAngles(-90, 0, 0)
            scale: Qt.vector3d(100, 100, 100)
            source: "../shared/models/barrel/meshes/Barrel.mesh"

            PrincipledMaterial {
                id: barrel_001
                baseColor: Qt.rgba(1.0, 0.639994, 0.639994, 1)
                metalness: 1.0
                roughness: 0.2
            }
            materials: [barrel_001]
        }
    }

    Model {
        id: cylinder
        position: Qt.vector3d(255.743, -27.1591, 185)
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        source: "#Cylinder"

        PrincipledMaterial {
            id: default_001
            opacity: 0.76
            metalness: 0.0
            roughness: 0.0
            baseColor: "orange"
            specularAmount: 0.8
            specularTint: 1.0
        }
        materials: [default_001]
    }
}
