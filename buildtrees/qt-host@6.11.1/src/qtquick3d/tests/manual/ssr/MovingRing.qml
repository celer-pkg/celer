// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Model {
    id: model

    geometry: TorusGeometry {}

    property variant material
    materials: [ material ]

    property bool animate: true

    SequentialAnimation {
        running: model.animate
        loops: Animation.Infinite
        NumberAnimation  { target: model; property: "position.y"; from: 0; to: -90; duration: 5000; }
        NumberAnimation  { target: model; property: "position.y"; from: -90; to: 0; duration: 5000; }
    }
}
