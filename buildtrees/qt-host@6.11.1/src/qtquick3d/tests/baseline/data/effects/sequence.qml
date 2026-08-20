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
        id: myNoise2
        source: "../shared/maps/checkers2.png"
        tilingModeHorizontal: Texture.Repeat
        tilingModeVertical: Texture.Repeat
    }

    Texture {
        id: myNoise3
        source: "../shared/maps/checkers2.png"
        tilingModeHorizontal: Texture.Repeat
        tilingModeVertical: Texture.Repeat
    }

    Desaturate { id: e0a; amount: 1 }
    ColorMaster { id: e0b; redStrength: 0; greenStrength: 2; blueStrength: 0 }

    ColorMaster { id: e1a; redStrength: 0; greenStrength: 2; blueStrength: 0 }
    Desaturate { id: e1b; amount: 1 }

    Scatter { id: e2a; randomize: false; noiseSample: TextureInput { texture: myNoise2 } }
    DistortionSpiral { id: e2b }
    Emboss { id: e2c }

    Emboss { id: e3a }
    DistortionSpiral { id: e3b }
    Scatter { id: e3c; randomize: false; noiseSample: TextureInput { texture: myNoise3 } }

    delegate: PlainView {
        effect: index == 0 ? [ e0a, e1b ] :
                index == 1 ? [ e1a, e1b ] :
                index == 2 ? [ e2a, e2b, e2c ] :
                             [ e3a, e3b, e3c ]
    }
}
