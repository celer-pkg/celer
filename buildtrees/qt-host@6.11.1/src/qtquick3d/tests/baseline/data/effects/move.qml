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

    MotionBlur { id: e0 }
    MotionBlur { id: e1; fadeAmount: 1 }
    MotionBlur { id: e2; fadeAmount: 0 }
    MotionBlur { id: e3; fadeAmount: 0.1; blurQuality: 1 }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
        animated: true
    }
}
