// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: node
        property real yOffset: 11
        property real width: 80
        property real height: 50

    // Resources
    PrincipledMaterial {
        id: principledMaterial
        roughness: 0.5
        alphaMode: PrincipledMaterial.Opaque
        baseColor: "black"
    }

    // Nodes:
    Model {
        id: monitor
        eulerRotation.y: -90
        objectName: "Monitor"
        source: "meshes/monitor_mesh.mesh"
        materials: [
            principledMaterial
        ]
    }

    // Animations:
}
