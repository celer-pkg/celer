// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

NumberAnimation {
    property string mode: "ZoomIn"
    property: "scale"
    duration: 1000
    easing.amplitude: 6.0
    easing.period: 2.5
    from: mode == "ZoomIn" ? 0.3 : 1.0
    to: mode == "ZoomIn" ? 1.0 : 0.3
}
