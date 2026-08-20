// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
//
import QtQuick
import QtQuick.Layouts
import QtQuick3D.Helpers
import QtQuick3D
import QtQuick3D.Xr

Node {
    DirectionalLight {
    }

    Model {
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColor: "green"
        }
        position: Qt.vector3d(25, 0, 0)
        scale: Qt.vector3d(0.5, 0.5, 0.5)
    }

    Model {
        source: "#Sphere"
        materials: PrincipledMaterial {
            baseColor: "blue"
        }
        position: Qt.vector3d(-25, 0, 0)
        scale: Qt.vector3d(0.5, 0.5, 0.5)
    }
}
