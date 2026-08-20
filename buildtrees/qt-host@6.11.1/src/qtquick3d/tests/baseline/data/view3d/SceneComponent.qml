// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 200)
    }
    DirectionalLight {
    }
    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseColor: "green"
        }
        eulerRotation: Qt.vector3d(45, 45, 45)
    }
}
