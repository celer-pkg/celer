// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

DynamicRigidBody {
    property bool inArea: false
    property real xyzExtents: 1
    massMode: DynamicRigidBody.CustomDensity
    density: 10

    Model {
        source: "#Cube"
        scale: Qt.vector3d(xyzExtents, xyzExtents, xyzExtents).times(0.01)
        materials: PrincipledMaterial {
            baseColor: "red"
        }
    }

    physicsMaterial: PhysicsMaterial {
        restitution: 0.6
        dynamicFriction: 0.5
        staticFriction: 0.5
    }

    collisionShapes: BoxShape {
        extents: Qt.vector3d(xyzExtents, xyzExtents, xyzExtents)
    }
}
