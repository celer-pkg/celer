// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Xr

XrController {
    controller: XrController.ControllerRight
    poseSpace: XrController.AimPose
    Model {
        source: "#Cube"
        scale: Qt.vector3d(0.1, 0.1, 0.1)
        materials: DefaultMaterial {
            diffuseColor: "red"
            lighting: DefaultMaterial.NoLighting
        }
    }
}
