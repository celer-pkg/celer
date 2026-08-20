// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs

GraphsView {
    width: 300
    height: 300

    SplineSeries {
        id: splineSeries
        XYPoint { x: 1; y: 1 }
        XYPoint { x: 2; y: 4 }
        XYPoint { x: 4; y: 2 }
        XYPoint { x: 5; y: 5 }
    }
}
