// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs

GraphsView {
    width: 300
    height: 300

    BarSeries {
        id: barSeries
        BarSet { id: set1; label: "Set1"; values: [2, 2, 3] }
        BarSet { id: set2; label: "Set2"; values: [5, 1, 2] }
        BarSet { id: set3; label: "Set3"; values: [3, 5, 8] }
    }
}
