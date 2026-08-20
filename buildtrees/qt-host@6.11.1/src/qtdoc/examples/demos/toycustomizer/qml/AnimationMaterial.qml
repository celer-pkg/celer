// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

PrincipledMaterial {
    id: animationMaterial

    property string materialName
    property string baseColorSource
    property string normalMapSource

    objectName: materialName
    baseColorMap: baseColorTexture
    normalMap: normalMapTexture
    baseColor: "#ffffff"
    roughness: 1
    normalStrength: 0.4000000059604645
    cullMode: PrincipledMaterial.NoCulling
    alphaMode: PrincipledMaterial.Opaque

    Texture {
        id: baseColorTexture
        source: animationMaterial.baseColorSource
    }

    Texture {
        id: normalMapTexture
        source: animationMaterial.normalMapSource
    }
}
