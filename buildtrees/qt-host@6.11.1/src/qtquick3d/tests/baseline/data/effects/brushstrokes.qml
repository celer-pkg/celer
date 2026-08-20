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
        source: "../shared/maps/opacitymap.png"
        tilingModeHorizontal: Texture.Repeat
        tilingModeVertical: Texture.Repeat
    }
    BrushStrokes { id: e0 }
    BrushStrokes { id: e1; brushLength: 3 }
    BrushStrokes { id: e2; brushSize: 20; brushAngle: -90 }
    BrushStrokes { id: e3; noiseSample: TextureInput { texture: myNoise } }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
