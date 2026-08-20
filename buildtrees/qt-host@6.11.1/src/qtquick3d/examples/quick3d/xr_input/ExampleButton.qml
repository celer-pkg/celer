// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D


Model {
    property bool on: false
    required property real rotationSpeed

    scale: Qt.vector3d(0.15, 0.1, 0.1)
    z: on ? -3 : 0
    source: "#Cube"
    materials: PrincipledMaterial {
        baseColor: "#111133"
    }
    Behavior on z { NumberAnimation {duration: 300} }
    pickable: true
}
