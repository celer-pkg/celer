// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

//! [logo model]
Model {
    id: logoDefault
    source: "qtlogo.mesh"
    scale: Qt.vector3d(5000, 5000, 5000)

    property variant material
    materials: [ material ]

    property bool animate: true
    NumberAnimation on eulerRotation.y {
        running: logoDefault.animate
        loops: Animation.Infinite
        duration: 5000
        from: 0
        to: -360
    }
}
//! [logo model]
