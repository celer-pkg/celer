// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

CustomMaterial {
    property vector3d minBounds
    property vector3d maxBounds
    property TextureInput textureSampler: TextureInput {}
    property TextureInput colorSampler: TextureInput {}
    property int color8Bit
    property vector3d textureDimensions
    property int sampleCount
    property real alphaMultiplier
    property int preserveOpacity
    property bool useOrtho

    shadingMode: CustomMaterial.Unshaded
    sourceBlend: CustomMaterial.SrcAlpha
    destinationBlend: CustomMaterial.OneMinusSrcAlpha
    vertexShader: "qrc:/shaders/texture3dvert"
    fragmentShader: "qrc:/shaders/texture3dfrag"
}
