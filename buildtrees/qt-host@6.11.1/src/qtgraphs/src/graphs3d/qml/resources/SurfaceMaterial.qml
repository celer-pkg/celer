// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

CustomMaterial {
    property TextureInput custex: TextureInput {}
    property TextureInput height: TextureInput {}
    property TextureInput baseColor: TextureInput {}

    property real xDiff: 0.0
    property real yDiff: 0.0
    property real graphHeight
    property vector2d uvOffset
    property vector2d size
    property vector2d vertCount
    property real rootScale

    property real gradientMin
    property real gradientHeight
    property color uniformColor
    property bool textured: false
    property bool flatShading: false
    property int colorStyle: 0
    property bool fill: false
    property bool lineData: false

    property bool flipU: false
    property bool flipV: false

    property bool hasTransparency
    property real order: 0

    property real specularBrightness: 0.25
    readonly property real shininess: (1.0 - specularBrightness) * 100

    property bool shaded: false

    shadingMode: CustomMaterial.Shaded
    vertexShader: "qrc:/shaders/surfacevert"
    fragmentShader: "qrc:/shaders/surfacefrag"
    sourceBlend: hasTransparency? CustomMaterial.SrcAlpha : CustomMaterial.NoBlend
    destinationBlend: hasTransparency? CustomMaterial.OneMinusSrcAlpha : CustomMaterial.NoBlend
}
