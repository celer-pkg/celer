// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

//! [implementation]
CustomMaterial {
    id: root
    property real time: 0.0
    property real amplitude: 5.0
    property real alpha: 1.0

    property bool texturing: false
    property bool textureFromItem: false
    property Item texSrc
    Texture {
        id: texFromFile
        source: "qt_logo.png"
    }
    Texture {
        id: texFromItem
        sourceItem: root.texSrc
    }
    property TextureInput tex: TextureInput {
        enabled: root.texturing
        texture: root.textureFromItem ? texFromItem : texFromFile
    }

    shadingMode: CustomMaterial.Unshaded
    sourceBlend: root.alpha < 1.0 ? CustomMaterial.SrcAlpha : CustomMaterial.NoBlend
    destinationBlend: root.alpha < 1.0 ? CustomMaterial.OneMinusSrcAlpha : CustomMaterial.NoBlend
    cullMode: CustomMaterial.BackFaceCulling

    vertexShader: "example.vert"
    fragmentShader: root.texturing ? "example_tex.frag" : "example.frag"
}
//! [implementation]
