// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick3D
import QtQuick

CustomMaterial {
    property double rayMaxDistance: 100
    property int marchSteps: 100
    property int refinementSteps: 10
    property double depthBias: 0.3
    property double specular: 0.3
    property color materialColor: "transparent"

    sourceBlend: CustomMaterial.One
    destinationBlend: CustomMaterial.OneMinusSrcAlpha
    fragmentShader: "material_screenspacereflections.frag"
}
