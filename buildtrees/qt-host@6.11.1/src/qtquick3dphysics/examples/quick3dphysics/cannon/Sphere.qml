// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

DynamicRigidBody {
    property bool inArea: false
    property real sphereDiameter: 2
    massMode: DynamicRigidBody.CustomDensity
    density: 10

    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1, 1, 1).times(sphereDiameter * 0.01)
        materials: PrincipledMaterial {
            baseColor: "yellow"
        }
    }

    physicsMaterial: PhysicsMaterial {
        restitution: 0.6
        dynamicFriction: 0.5
        staticFriction: 0.5
    }

    collisionShapes: SphereShape {
        diameter: sphereDiameter
    }
}
