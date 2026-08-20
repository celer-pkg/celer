// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma Singleton

import QtQuick
import QtQuick3D

PrincipledMaterial {
    Texture {
        id: campfire_bc
        generateMipmaps: true
        source: "media/textures/campfire/campfire_bc.jpg"
    }
    Texture {
        id: campfire_mr
        generateMipmaps: true
        source: "media/textures/campfire/campfire_mr.jpg"
    }
    Texture {
        id: campfire_n
        generateMipmaps: true
        source: "media/textures/campfire/campfire_n.jpg"
    }

    baseColorMap: campfire_bc
    metalnessMap: campfire_mr
    metalnessChannel: PrincipledMaterial.R
    roughnessMap: campfire_mr
    roughnessChannel: PrincipledMaterial.G
    metalness: 1
    roughness: 1
    normalMap: campfire_n
    occlusionMap: campfire_mr
    occlusionChannel: PrincipledMaterial.B
}

