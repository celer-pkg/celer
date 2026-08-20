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

    TiltShift { id: e0 }
    TiltShift { id: e1; focusPosition: 0.2; focusWidth: 0.1; blurAmount: 8; isVertical: true; isInverted: true }
    Fxaa { id: e2 }
    MotionBlur { id: e3 }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
