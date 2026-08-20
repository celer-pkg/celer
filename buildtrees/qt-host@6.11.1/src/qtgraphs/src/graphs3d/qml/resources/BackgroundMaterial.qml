// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick3D

CustomMaterial {
    property TextureInput gridTex: TextureInput {}
    property vector3d scale
    property vector3d rootScale
    property vector3d margin
    property color gridLineColor
    property color subgridLineColor
    property color baseColor
    property real gridWidth: 0.25
    property bool polar
    property bool xCategory
    property bool zCategory

    property bool gridVisible: true
    property bool baseVisible: true
    property bool gridOnTop: false

    shadingMode: CustomMaterial.Shaded
    cullMode: CustomMaterial.NoCulling
    sourceBlend: baseVisible? CustomMaterial.NoBlend : CustomMaterial.SrcAlpha
    destinationBlend: baseVisible? CustomMaterial.NoBlend : CustomMaterial.OneMinusSrcAlpha
    fragmentShader: "qrc:/shaders/backgroundgridfrag"
}
