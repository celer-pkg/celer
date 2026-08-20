// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma Singleton

import QtQuick
import QtQuick3D

CustomMaterial {
    shadingMode: CustomMaterial.Unshaded
    cullMode: Material.BackFaceCulling
    vertexShader: "media/shaders/invisible.vert"
    fragmentShader: "media/shaders/invisible.frag"
}

