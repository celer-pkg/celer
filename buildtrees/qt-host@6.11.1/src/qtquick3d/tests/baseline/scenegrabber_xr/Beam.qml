// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Model {
    source: "#Cylinder"
    scale: Qt.vector3d(0.0001, 1, 0.0001)
    eulerRotation.x: -90
    z: -50

    materials: PrincipledMaterial {
        baseColor: "green"
        opacity: 0.5
        metalness: 0
        roughness: 0.1
        lighting: PrincipledMaterial.NoLighting
    }
}
