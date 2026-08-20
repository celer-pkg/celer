// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

PrincipledMaterial {
    id: asphalt
    cullMode: Material.NoCulling
    roughnessMap: asphalt010_2K_Roughness
    normalMap: asphalt010_2K_NormalGL
    roughness: 1
    objectName: "Asphalt"
    baseColor: "#404040"
    specularAmount: 1
    clearcoatAmount: 0
    metalness: 0

    Texture {
        id: asphalt010_2K_NormalGL
        source: "file:content/images/Asphalt010_2K_NormalGL.png"
        scaleV: 3
        scaleU: 3
    }

    Texture {
        id: asphalt010_2K_Roughness
        source: "file:content/images/Asphalt010_2K_Roughness.png"
        scaleV: 3
        scaleU: 3
    }
}
