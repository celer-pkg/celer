// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

CustomMaterial {
    property TextureInput custex: TextureInput {}
    property int colorStyle
    property color uniformColor
    property bool isHighlight
    property bool transparency: false

    property bool valueColoring
    property real heightValue
    property real rootScale

    property real specularBrightness: 0.25
    readonly property real shininess: (1.0 - specularBrightness) * 100

    property bool shaded: false

    shadingMode: CustomMaterial.Shaded
    sourceBlend: !transparency ? CustomMaterial.NoBlend : CustomMaterial.SrcAlpha
    destinationBlend: !transparency ? CustomMaterial.NoBlend : CustomMaterial.OneMinusSrcAlpha
    depthDrawMode: !transparency ? Material.OpaqueOnlyDepthDraw : Material.OpaquePrePassDepthDraw

    vertexShader: "qrc:/shaders/objectgradientvert"
    fragmentShader: "qrc:/shaders/barsfrag"
}
