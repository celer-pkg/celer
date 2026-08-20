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

    AdditiveColorGradient { id: e0 }
    AdditiveColorGradient { id: e1; bottomColor: Qt.vector3d(0.4, 0, 0.1); topColor: Qt.vector3d(0.2, 0.2, 0) }
    ColorMaster { id: e2 }
    ColorMaster { id: e3; greenStrength: 0; saturation: 0.6 }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : e3
    }
}
