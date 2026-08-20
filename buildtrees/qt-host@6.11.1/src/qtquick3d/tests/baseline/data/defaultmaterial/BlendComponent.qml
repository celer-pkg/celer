// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Node {
    id: componentRoot

    property var blendMode: DefaultMaterial.Screen

    Model {
        position: Qt.vector3d(-10, -10, 0)
        scale: Qt.vector3d(2,2,0.05)
        opacity: 0.5
        source: "#Cube"
        materials: DefaultMaterial {
            blendMode: componentRoot.blendMode
            diffuseColor: Qt.rgba(1.0, 0.0, 0.0, 1)
        }
    }
    Model {
        position: Qt.vector3d(10, 10, 10)
        scale: Qt.vector3d(2,2,0.05)
        opacity: 0.5
        source: "#Cube"
        materials: DefaultMaterial {
            blendMode: componentRoot.blendMode
            diffuseColor: Qt.rgba(0.0, 1.0, 0.0, 1)
        }
    }
    Model {
        id: cone
        position: Qt.vector3d(-30, 30, 50)
        scale: Qt.vector3d(1,1,1)
        source: "#Cone"
        materials: DefaultMaterial {
            blendMode: componentRoot.blendMode
            diffuseColor: Qt.rgba(0.0, 0.0, 1.0, 1)
        }
    }
    Model {
        id: cylinder
        position: Qt.vector3d(30, -30, 100)
        scale: Qt.vector3d(1,1,1)
        opacity: 0.5
        source: "#Cylinder"
        materials: DefaultMaterial {
            blendMode: componentRoot.blendMode
            diffuseColor: Qt.rgba(0.0, 1.0, 1.0, 1)
        }
    }
}
