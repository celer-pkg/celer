// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

CustomMaterial {
    property vector3d lightPosition_wrld

    shadingMode: CustomMaterial.Unshaded
    fragmentShader: "qrc:/shaders/fragmentSurfaceShadowNoTex"
}
