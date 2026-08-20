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

    HDRBloomTonemap { id: e0 }
    HDRBloomTonemap { id: e1; bloomThreshold: 0.8; blurFalloff: 2 }
    SCurveTonemap { id: e2;  }
    SCurveTonemap { id: e3; contrastBoost: 2; whitePoint: 4.2; exposureValue: 4 }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
