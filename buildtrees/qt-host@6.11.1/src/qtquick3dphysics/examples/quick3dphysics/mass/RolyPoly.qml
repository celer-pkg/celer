// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

//! [rolypoly]
DynamicRigidBody {
    property string color: "blue"

    collisionShapes: [
        SphereShape {
            id: sphere0
            diameter:  1
        },
        SphereShape {
            id: sphere1
            diameter:  0.8
            position: Qt.vector3d(0, 0.6, 0)
        },
        SphereShape {
            id: sphere2
            diameter:  0.6
            position: Qt.vector3d(0, 1.1, 0)
        }
    ]

    Model {
        source: "#Sphere"
        position: sphere0.position
        scale: Qt.vector3d(1,1,1).times(sphere0.diameter*0.01)
        materials: PrincipledMaterial {
            baseColor: color
        }
    }

    Model {
        source: "#Sphere"
        position: sphere1.position
        scale: Qt.vector3d(1,1,1).times(sphere1.diameter*0.01)
        materials: PrincipledMaterial {
            baseColor: color
        }
    }

    Model {
        source: "#Sphere"
        position: sphere2.position
        scale: Qt.vector3d(1,1,1).times(sphere2.diameter*0.01)
        materials: PrincipledMaterial {
            baseColor: color
        }
    }
}
//! [rolypoly]
