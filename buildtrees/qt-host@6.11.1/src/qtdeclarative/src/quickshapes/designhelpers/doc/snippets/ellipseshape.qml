// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Shapes

Window {
    visible: true
    flags: Qt.FramelessWindowHint
    width: 100
    height: 100

//! [ellipseShape]
    EllipseShape {
        id: ellipseShape
        anchors.fill: parent
        width: 90
        height: 90
        startAngle: 0
        sweepAngle: 270
    }
//! [ellipseShape]
}
