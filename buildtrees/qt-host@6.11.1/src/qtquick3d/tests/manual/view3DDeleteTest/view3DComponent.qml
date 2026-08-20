// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    Node {
        DirectionalLight {
        }

        PerspectiveCamera {
            z: 550
        }

        Model {
            source: "#Sphere"
            materials: [ theMaterial ]
        }
        DefaultMaterial {
            id: theMaterial
            diffuseColor: "blue"
        }
    }
}
