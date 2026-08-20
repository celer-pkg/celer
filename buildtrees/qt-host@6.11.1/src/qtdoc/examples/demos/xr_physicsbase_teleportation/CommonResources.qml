// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma Singleton

import QtQuick
import QtQuick3D

Node {
    property alias turbulenceTexture: turbulence_texture

    Texture {
        id: turbulence_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "media/textures/turbulence.png"
    }
}

