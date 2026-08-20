// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Physics

//! [capsulelink]
DynamicRigidBody {
    property real len: 170
    property real w: 17
    PrincipledMaterial {
        id: material3
        baseColor: "yellow"
        metalness: 1.0
        roughness: 0.5
    }
    Node {
        opacity: 1
        Model {
            materials: material3
            source: "#Cylinder"
            scale: Qt.vector3d(w / 100, len / 100, w / 100)
            eulerRotation.z: 90
            y: -len / 2
        }
        Model {
            materials: material3
            source: "#Cylinder"
            scale: Qt.vector3d(w / 100, len / 100, w / 100)
            eulerRotation.z: 90
            y: len / 2
        }
        Model {
            materials: material3
            source: "#Cylinder"
            scale: Qt.vector3d(w / 100, len / 100, w / 100)
            x: len / 2
        }
        Model {
            materials: material3
            source: "#Cylinder"
            scale: Qt.vector3d(w / 100, len / 100, w / 100)
            x: -len / 2
        }
        Model {
            materials: material3
            source: "#Sphere"
            scale: Qt.vector3d(w / 100, w / 100, w / 100)
            x: -len / 2
            y: -len / 2
        }
        Model {
            materials: material3
            source: "#Sphere"
            scale: Qt.vector3d(w / 100, w / 100, w / 100)
            x: -len / 2
            y: len / 2
        }
        Model {
            materials: material3
            source: "#Sphere"
            scale: Qt.vector3d(w / 100, w / 100, w / 100)
            x: len / 2
            y: -len / 2
        }
        Model {
            materials: material3
            source: "#Sphere"
            scale: Qt.vector3d(w / 100, w / 100, w / 100)
            x: len / 2
            y: len / 2
        }
    }
    collisionShapes: [
        CapsuleShape {
            y: -len / 2
            height: len
            diameter: w
        },
        CapsuleShape {
            y: len / 2
            height: len
            diameter: w
        },
        CapsuleShape {
            x: -len / 2
            eulerRotation.z: 90
            height: len
            diameter: w
        },
        CapsuleShape {
            x: len / 2
            eulerRotation.z: 90
            height: len
            diameter: w
        }
    ]
}
//! [capsulelink]
