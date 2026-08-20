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
    DepthOfFieldHQBlur { id: e0 }
    DepthOfFieldHQBlur { id: e1; blurAmount: 10 }
    DepthOfFieldHQBlur { id: e2; focusDistance: 300 }
    DepthOfFieldHQBlur { id: e3; focusDistance: 300; focusRange: 10 }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
