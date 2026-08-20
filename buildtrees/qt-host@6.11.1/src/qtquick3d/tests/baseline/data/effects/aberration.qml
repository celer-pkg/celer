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
    ChromaticAberration { id: e0 }
    ChromaticAberration { id: e1; maskTexture: TextureInput { texture: myNoise } }
    ChromaticAberration { id: e2; focusDepth: 200 }
    ChromaticAberration { id: e3; aberrationAmount: 10 }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
