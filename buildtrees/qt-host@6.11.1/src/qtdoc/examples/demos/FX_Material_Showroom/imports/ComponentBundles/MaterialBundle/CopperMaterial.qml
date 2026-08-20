// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

PrincipledMaterial {
    id: copper
    cullMode: Material.NoCulling
    roughnessMap: roughness
    baseColor: "#fad0c0"
    objectName: "Copper"
    metalness: 1
    roughness: 0.15
    clearcoatAmount: 0

    Texture {
        id: roughness
        source: "images/Metal029_2K_Displacement.jpg"
        mipFilter: Texture.Linear
        scaleV: 5
        scaleU: 5
    }
}
