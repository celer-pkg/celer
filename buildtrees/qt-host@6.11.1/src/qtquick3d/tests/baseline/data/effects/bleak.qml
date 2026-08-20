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

    Desaturate { id: e0 }
    Desaturate { id: e1; amount: 0.8 }
    Vignette { id: e2 }
    Vignette { id: e3; vignetteColor: Qt.vector3d(0, 0.8, 0.5); vignetteRadius: 1 }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
