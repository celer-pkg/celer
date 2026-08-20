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

    DistortionRipple { id: e0 }
    DistortionRipple { id: e1; center: Qt.vector2d(0.4, 0.8); distortionPhase: 60 }
    DistortionSphere { id: e2 }
    DistortionSphere { id: e3; center: Qt.vector2d(0.4, 0.8); distortionHeight: -1 }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
