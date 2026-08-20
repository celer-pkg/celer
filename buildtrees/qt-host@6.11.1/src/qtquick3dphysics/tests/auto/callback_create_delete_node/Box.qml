// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Physics

DynamicRigidBody {
    Model {
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColor: "red"
        }
    }

    sendContactReports: true
    receiveContactReports: true
    onBodyContact: (body, positions, impulses, normals) => {}

    collisionShapes: BoxShape {}
}
