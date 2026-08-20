// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D.Physics

StaticRigidBody {
    // Model table radius is 0.482967cm. Goal radius is 30 cm
    scale: Qt.vector3d(1, 1, 1).times(30 / 0.482967)
    eulerRotation: Qt.vector3d(-90, 0, 0)
    position: Qt.vector3d(0, -50, 0)
    sendContactReports: true
    collisionShapes: TriangleMeshShape {
        source: "meshes/side_Table_Pine_LOD0_M_Side_Table_Natural_Wenge_Wood_4K_0.mesh"
    }
    RoundTable {}
}
