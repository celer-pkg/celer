// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Component {
    Model {
        property color seriesColor: "white"
        property int seriesItem: index
        source: "#Sphere"
        pickable: true
        scale: Qt.vector3d(0.001, 0.001, 0.001)
    }
}
