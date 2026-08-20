// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

DynamicRigidBody {
    property bool inArea: false
    mass: 10
    position: Qt.vector3d(1, 2, 0)

    sendContactReports: true
    sendTriggerReports: true

    Model {
        source: "#Sphere"
        materials: PrincipledMaterial {
            baseColor: inArea ? "blue" : "red"
        }
        scale: Qt.vector3d(0.01, 0.01, 0.01)
    }

    collisionShapes: SphereShape {
        diameter: 1
    }
}
