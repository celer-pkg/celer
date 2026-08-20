// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

GridView {
    width: 400
    height: 400
    cellWidth: 200
    cellHeight: 200

    model: 4

    Texture {
        id: myNoise
        source: "../shared/maps/checkers2.png"
        tilingModeHorizontal: Texture.Repeat
        tilingModeVertical: Texture.Repeat
    }

    DistortionSpiral { id: e0 }
    DistortionSpiral { id: e1; center: Qt.vector2d(0.4, 0.8); distortionStrength: -10 }
    Scatter { id: e2; randomize: false }
    Scatter { id: e3; randomize: false; amount: 40; direction: 2; noiseSample: TextureInput { texture: myNoise } }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
